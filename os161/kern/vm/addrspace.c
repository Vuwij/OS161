#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/spl.h>
#include <machine/tlb.h>
#include <coremap.h>
#include <pagedirectory.h>
#include <swapmap.h>
#include <page.h>
#include <vfs.h>
#include <kern/unistd.h>
#include <linkedlist.h>

#include "vnode.h"
#include "synch.h"


#define TEXT_SEGMENT_SHIFT 16
#define MAX_STACK_GROWTH 0x10000000
#define PRINTVM 0
#define DEBUG_COPY 0
#define DEBUG_COPY_ON_WRITE 0

// For guarding the page directory temporarily

struct lock* copy_on_write_lock;

void
vm_bootstrap(void) {
    copy_on_write_lock = lock_create("Copy on Write");
}

/* Allocate/free some user-space virtual pages */
paddr_t
alloc_upages(int npages, vaddr_t vaddr) {
    int spl;

    paddr_t addr;

    spl = splhigh();
    addr = ram_borrowmemuser(npages, curthread->pid, vaddr);
    splx(spl);

    if (addr == 0) {
        return 0;
    }
    return addr;
}

void
zero_upages(vaddr_t addr) {
    int spl = splhigh();
    ram_zeromem(curthread->pid, addr);
    splx(spl);
}

void
increment_frame(int frame) {
    int spl = splhigh();
    ram_incrementframe(frame - (firstpaddr >> 12));
    splx(spl);
}

// Provides the frame number of the physical address
// Example if frame is 0x42. Physical address is 0x42000. Index on coremap is 0x42000 - first physical address

void
free_frame(int frame) {
    int spl = splhigh();
    ram_removeframe(frame - (firstpaddr >> 12));
    splx(spl);
}

/* Allocate/free some kernel-space virtual pages */
vaddr_t
alloc_kpages(int npages) {
    paddr_t addr;

    int spl = splhigh();
    addr = ram_borrowmem(npages);
    splx(spl);

    if (addr == 0) {
        return 0;
    }
    return PADDR_TO_KVADDR(addr);
}

void
free_kpages(vaddr_t addr) {
    DEBUG(DB_VM, "  free_kpages: 0x%x\n", addr);

    // Remove the memory from the core map
    int spl = splhigh();
    ram_returnmem(addr);
    splx(spl);
}

int
vm_fault(int faulttype, vaddr_t faultaddress) {
    paddr_t paddr;
    int i;
    u_int32_t ehi, elo;
    struct addrspace *as;
    int spl;

    spl = splhigh();

    faultaddress &= PAGE_FRAME;

    switch (faulttype) {
        case VM_FAULT_READONLY:
            /* We always create pages read-write, so we can't get this */
            panic("dumbvm: got VM_FAULT_READONLY\n");
        case VM_FAULT_READ:
        case VM_FAULT_WRITE:
            break;
        default:
            goto tlbfault;
    }

    as = curthread->t_vmspace;
    if (as == NULL) {
        kprintf("TLB: Address Space is Null\n");
        goto tlbfault;
    }

    lock_acquire(as->pdlock);
    struct page* p = pd_request_page(&as->page_directory, faultaddress);
    lock_release(as->pdlock);
    
//    kprintf("PID %d [%d]\n", curthread->pid, faulttype);
//    if(curthread->pid == 2)
//        pd_translate(faultaddress);

    // Page in memory, all other cases have page not loaded
    if (p->V && p->PFN) {
                // Check if coremap contains a duplicate
        if (faulttype == VM_FAULT_WRITE /*|| faultaddress == as->as_data*/) {
            lock_acquire(copy_on_write_lock);
            struct coremap_entry* cmentry = cm_getcmentryfromaddress((p->PFN << 12));

            if (cmentry->usecount > 1) {
                
                // Hacky way to save memory
                //pd_copy_on_write(&as->page_directory, faultaddress);
                
                p->V = 1;
                p->R = 1;
                unsigned copyfrom = p->PFN;
                p->PFN = 0;
                p_allocate_page(p); // Allocate disk space for page
                sm_swapin(p, faultaddress); // Swap the page from the disk
                unsigned copyto = p->PFN;

                ram_copymem((copyto << 12), (copyfrom << 12));
                
                
                if(DEBUG_COPY_ON_WRITE) cm_print();
            }
            lock_release(copy_on_write_lock);
        }

        paddr = (p->PFN << 12);
        p->R = 1;
    }

    // Page on file, not loaded
    if (p->F && p->V == 0) {
        // If in data segment

        int segment = (p->PFN >> TEXT_SEGMENT_SHIFT);
        int part = p->PFN - (segment << TEXT_SEGMENT_SHIFT);

        p->V = 1;
        p->PFN = 0;
        p->F = 0;
        p_allocate_page(p); // Allocate disk space for page
        sm_swapin(p, faultaddress); // Swap the page from the disk

        paddr = (p->PFN << 12);
        load_elf_segment(segment, part);

        return 0;
    }

    // Page valid, unallocated
    if (p->F == 0 && p->V == 1 && p->PFN == 0) {
        p_allocate_page(p); // Allocate disk space for page
        sm_swapin(p, faultaddress); // Swap the page from the disk
        paddr = (p->PFN << 12);

        return 0;
    }

    // Page located on disk
    if (!p->V && p->PFN) {
        //        pd_print(&as->page_directory);
        sm_swapin(p, faultaddress);
        paddr = (p->PFN << 12);
    }

    // Page not allocated, allocate from stack
    if (p->PFN == 0 && p->V == 0) {
        if (faultaddress >= USERSTACK)
            goto tlbfault;

        // Fault location is part of stack (Temporary solution to stack growth method)
        if (faultaddress <= as->as_stacklocation && faultaddress > (as->as_stacklocation - MAX_STACK_GROWTH)) {
            unsigned addr = as->as_stacklocation - PAGE_SIZE;

            // Request pages for a jump
            while (addr > faultaddress) {
                struct page* pp = pd_request_page(&as->page_directory, addr);
                if (pp->PFN != 0 || pp->F != 0 || pp->V != 0) {
                    kprintf("Page has hit the bottom\n");
                    goto tlbfault; // Page has hit the bottom
                }
                pp->V = 1;
                pp->Prot = 3;
                addr = addr - PAGE_SIZE;
            }

            // The page that actually is requested
            struct page* pp = pd_request_page(&as->page_directory, faultaddress);
            if (pp->PFN != 0 || pp->F != 0 || pp->V != 0) {
                kprintf("Page has hit the bottom\n");
                goto tlbfault; // Page has hit the bottom
            }
            pp->V = 1;
            pp->Prot = 3;
            p_allocate_page(pp); // Allocate disk space for page
            sm_swapin(pp, faultaddress); // Swap the page from the disk
            paddr = (pp->PFN << 12);
            as->as_stacklocation = faultaddress; // Shrink the stack location, stack location is never freed

        } else if (faultaddress >= as->as_heap_start && faultaddress <= as->as_heap_end) {
            struct page* pp = pd_request_page(&as->page_directory, faultaddress);
            if (pp->PFN != 0 || pp->F != 0 || pp->V != 0) goto tlbfault;
            pp->V = 1;
            pp->Prot = 3;
            p_allocate_page(pp); // Allocate disk space for page
            sm_swapin(pp, faultaddress); // Swap the page from the disk
            paddr = (pp->PFN << 12);
        } else {
            kprintf("Invalid Page\n");
            goto tlbfault;
        }
    }

    // TLB Stuff
    assert((paddr & PAGE_FRAME) == paddr);
    for (i = 0; i < NUM_TLB; i++) {
        TLB_Read(&ehi, &elo, i);

        // If the physical page is valid
        if (elo & TLBLO_VALID) {
            continue;
        }

        // If the physical page is invalid, replace the TLB entry with the new page
        ehi = faultaddress;
        elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
        DEBUG(DB_VM, "  smartvm:%d 0x%x -> 0x%x\n", faulttype, faultaddress, paddr); // Prints all the mapping in the TLB
        TLB_Write(ehi, elo, i);
        splx(spl);
        return 0;
    }
    kprintf("Ran out of TLB entries - cannot handle page fault\n");
    goto tlbfault;

tlbfault:
    if (PRINTVM) {
        kprintf("-------------------- VM Fault Info --------------------\n");
        kprintf("PID %d\n", curthread->pid);
        kprintf("--- Page Info --- \n");
        kprintf("Virtual Address: 0x%x\n", faultaddress);
        pd_translate(faultaddress);
        kprintf("M: %d R: %d V: %d F: %d Prot: %d PFN 0x%x\n", p->M, p->R, p->V, p->F, p->Prot, p->PFN);
        kprintf("--- COREMAP --- \n");
        cm_print();
        kprintf("--- PAGE Directory --- \n");
        pd_print(&as->page_directory);
        kprintf("------------------------------------------------------\n");
    }
    splx(spl);
    return EFAULT;
}

struct addrspace *
as_create(void) {
    DEBUG(DB_VM, "as_create\n");

    struct addrspace *as = kmalloc(sizeof (struct addrspace));

    if (as == NULL) {
        return NULL;
    }

    as->pdlock = lock_create("Page Directory");
    pd_initialize(&as->page_directory);

    as->as_codeend = 0;
    as->as_codestart = 0;
    as->as_data = 0;
    as->as_heap_start = 0;
    as->as_heap_end = 0;
    as->as_stacklocation = 0;
    as->lruclock = NULL;
    as->lruhandle = NULL;

    return as;
}

void
as_reset(struct addrspace *as) {
    //    int spl = splhigh();
    //    kprintf("-------- AS Resetting for PID %d --------\n", curthread->pid);
    //    kprintf("Coremap 1\n");
    //    cm_print();
    //    kprintf("Page directory before\n");
    //    pd_print(&as->page_directory);

    // Destroy and recreate the address space
    vfs_close(as->progfile);

    lock_acquire(as->pdlock);
    pd_free(&as->page_directory);
    lock_release(as->pdlock);


    if (as == NULL) {
        return;
    }

    pd_initialize(&as->page_directory);


    as->as_codeend = 0;
    as->as_codestart = 0;
    as->as_data = 0;
    as->as_heap_start = 0;
    as->as_heap_end = 0;
    as->as_stacklocation = 0;
    as->lruclock = NULL;
    as->lruhandle = NULL;

    //    kprintf("Coremap After\n");
    //    cm_print();
    //    kprintf("--------  AS Reset for PID %d\n --------", curthread->pid);
    //    splx(spl);
}

void
as_destroy(struct addrspace *as) {
    DEBUG(DB_VM, "as_destroy\n");

    vfs_close(as->progfile);

    //    int spl = splhigh();
    //    kprintf("\n------------------------- Destroy for PID %d -------------------------\n", curthread->pid);
    //    pd_print(&as->page_directory);
    //    kprintf("Core Map Before\n");
    //    cm_print();

    lock_acquire(as->pdlock);

    ll_destroy(&as->lruclock);
    pd_free(&as->page_directory);

    lock_release(as->pdlock);
    lock_destroy(as->pdlock);
    kfree(as);

    //    kprintf("Core Map After\n");
    //    cm_print();
    //    kprintf("----------------------------------------------------------------------\n");
    //    splx(spl);
    //    as = NULL;
}

void
as_activate(struct addrspace *as) {
    int i, spl;

    (void) as;

    spl = splhigh();

    // Flushes the entire TLB
    for (i = 0; i < NUM_TLB; i++) {
        TLB_Write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
    }

    splx(spl);
}

/*
 * Set up a segment at virtual address VADDR of size MEMSIZE. The
 * segment in memory extends from VADDR up to (but not including)
 * VADDR+MEMSIZE.
 *
 * The READABLE, WRITEABLE, and EXECUTABLE flags are set if read,
 * write, or execute permission should be set on the segment. At the
 * moment, these are ignored. When you write the VM system, you may
 * want to implement them.
 */
int
as_define_region(struct addrspace *as, vaddr_t vaddr, int pindex, size_t sz,
        int readable, int writeable, int executable) {
    /*
     * Write this.
     */
    //    kprintf("as_define_region 0x%x, Size %d, Flags RWX: %d%d%d\n", vaddr, sz, readable, writeable, executable);

    size_t npages;

    /* Align the region. First, the base... */
    sz += vaddr & ~(vaddr_t) PAGE_FRAME;
    vaddr &= PAGE_FRAME;

    /* ...and now the length. */
    sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;

    npages = sz / PAGE_SIZE;

    // Get the page and set the valid to one

    // The page frame number temporarily stores the index of the code region and the shift in the text segment. This enables maps
    // the code into the individual segments in the code using pindex and i
    assert(npages < (1 << TEXT_SEGMENT_SHIFT));
    unsigned i;
    vaddr_t data = vaddr;
    lock_acquire(as->pdlock);
    for (i = 0; i < npages; i++) {
        struct page* p = pd_request_page(&as->page_directory, data);
        p->F = 1; // Indicates that the page is on file
        data = data + PAGE_SIZE;
        p->PFN = (pindex << TEXT_SEGMENT_SHIFT) + i;
        p->Prot = (readable >> 1) + (writeable >> 1);
    }

    as->as_data = vaddr;
    as->as_heap_start = data;
    as->as_heap_end = data;
    lock_release(as->pdlock);
    //    pd_print(&as->page_directory);
    return 0;
}

int
as_prepare_load(struct addrspace *as) {

    DEBUG(DB_VM, "as_prepare_load\n");

    // Setup the USER STACK pages
    pd_request_page(&as->page_directory, USERSTACK - PAGE_SIZE);
    as->as_stacklocation = USERSTACK - PAGE_SIZE;

    return 0;
}

int
as_complete_load(struct addrspace *as) {
    DEBUG(DB_VM, "as_complete_load\n");
    (void) as;
    return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr) {

    DEBUG(DB_VM, "as_define_stack\n");

    /* Initial user-level stack pointer */
    *stackptr = USERSTACK;

    (void) as;
    return 0;
}

int
as_copy(struct addrspace *old, struct addrspace **ret) {
    DEBUG(DB_VM, "as_copy\n");

    struct addrspace *newas = as_create();
    if (newas == NULL) {
        return ENOMEM;
    }

    strcpy(newas->progname, old->progname);

    // New address space must increase VOP_OPEN
    vfs_open(newas->progname, O_RDONLY, &newas->progfile);
    newas->eh = old->eh;

    newas->as_codestart = old->as_codestart;
    newas->as_codeend = old->as_codeend;
    newas->as_stacklocation = old->as_stacklocation;
    newas->as_heap_start = old->as_heap_start;
    newas->as_heap_end = old->as_heap_end;
    newas->as_data = old->as_data;

    // Make a copy of the page directory

    lock_acquire(copy_on_write_lock);
    lock_acquire(old->pdlock);

    int spl;
    if (DEBUG_COPY) {
        spl = splhigh();
        kprintf("------------ COPY from Thread %d --------------\n", curthread->pid);
        kprintf("Core Map Before\n");
        cm_print();
        splx(spl);
    }
    
    pd_copy(&newas->page_directory, &old->page_directory);

    if (DEBUG_COPY) {
        spl = splhigh();
        kprintf("Page Directory From: \n");
        pd_print(&old->page_directory);
        kprintf("Page Directory To: \n");
        pd_print(&newas->page_directory);
        kprintf("Core Map After\n");
        cm_print();
        kprintf("----------------------------------------------\n");
        splx(spl);
    }

    lock_release(old->pdlock);
    lock_release(copy_on_write_lock);

    // New AS needs page directory lock
    newas->pdlock = lock_create("Address Space Lock");

    *ret = newas;
    return 0;
}

void as_print(struct addrspace *as) {
    pd_print(&as->page_directory);
}
