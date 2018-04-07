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

#include "vnode.h"


#define TEXT_SEGMENT_SHIFT 16
#define MAX_STACK_GROWTH 0x10000000

void
vm_bootstrap(void) {
    /* Do nothing. */
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
free_upages(vaddr_t addr) {
    int spl = splhigh();
    ram_returnmemuser(curthread->pid, addr);
    splx(spl);
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
    ram_incrementframe((frame >> 12) - firstpaddr);
    splx(spl);
}

// Provides the frame number of the physical address
// Example if frame is 0x42. Physical address is 0x42000. Index on coremap is 0x42000 - first physical address
void
free_frame(int frame) {
    int spl = splhigh();
    ram_removeframe((frame >> 12) - firstpaddr);
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
            splx(spl);
            return EINVAL;
    }

    as = curthread->t_vmspace;
    if (as == NULL) return EFAULT;

    struct page* p = pd_request_page(&as->page_directory, faultaddress);


    // Page in memory
    if (p->V && p->PFN) {
        paddr = (p->PFN << 12);
    }
    
    // Page on file, not loaded
    if (p->F && p->V == 0) {
        // If in data segment
        
        int segment = (p->PFN >> TEXT_SEGMENT_SHIFT);
        int part = p->PFN - (segment << TEXT_SEGMENT_SHIFT);
        p->V = 1;
        p->PFN = 0;
        p_allocate_page(p); // Allocate disk space for page
        sm_swapin(p, faultaddress); // Swap the page from the disk
        
        if(faultaddress >= as->as_data && faultaddress < as->as_heap_end)
            zero_upages(faultaddress);
        
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
        sm_swapin(p, faultaddress);
        paddr = (p->PFN << 12);
    }
    
    // Page not allocated, allocate from stack
    if(p->PFN == 0 && p->V == 0){
        assert(faultaddress < USERSTACK);

        // Fault location is part of stack (Temporary solution to stack growth method)
        if (faultaddress <= as->as_stacklocation && faultaddress > (as->as_stacklocation - MAX_STACK_GROWTH)) {
            unsigned addr = as->as_stacklocation - PAGE_SIZE;
            
            // Request pages for a jump
            while(addr > faultaddress) {
                struct page* p = pd_request_page(&as->page_directory, addr);
                if (p->PFN != 0 || p->F != 0 || p->V != 0) return EFAULT; // Page has hit the bottom
                p->V = 1;
                addr = addr - PAGE_SIZE;
            }
            
            // The page that actually is requested
            struct page* p = pd_request_page(&as->page_directory, faultaddress);
            if (p->PFN != 0 || p->F != 0 || p->V != 0) return EFAULT; // Page has hit the bottom
            p->V = 1;
            p_allocate_page(p); // Allocate disk space for page
            sm_swapin(p, faultaddress); // Swap the page from the disk
            paddr = (p->PFN << 12);
            as->as_stacklocation = faultaddress; // Shrink the stack location, stack location is never freed
            
        } 
        else if (faultaddress >= as->as_heap_start && faultaddress <= as->as_heap_end) {
            struct page* p = pd_request_page(&as->page_directory, faultaddress);
            if (p->PFN != 0 || p->F != 0 || p->V != 0) return EFAULT;
            p->V = 1;
            p_allocate_page(p); // Allocate disk space for page
            sm_swapin(p, faultaddress); // Swap the page from the disk
            paddr = (p->PFN << 12);
        }
        
        else {
            return EFAULT;
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

    kprintf("dumbvm: Ran out of TLB entries - cannot handle page fault\n");
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
    
    pd_initialize(&as->page_directory);
    as->as_heap_start = 0;
    as->as_heap_end = 0;
    
    return as;
}

void
as_reset(struct addrspace *as) {
    DEBUG(DB_VM, "as_reset\n");
    if (as == NULL) {
        return;
    }
}

void
as_destroy(struct addrspace *as) {
    DEBUG(DB_VM, "as_destroy\n");

    //pd_print(&as->page_directory);
    cm_print();
    //sm_print();
    vfs_close(as->progfile);
    
    pd_free(&as->page_directory);
    kfree(as);
    as = NULL;
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
    DEBUG(DB_VM, "as_define_region 0x%x, Size %d, Flags RWX: %d%d%d\n", vaddr, sz, readable, writeable, executable);

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
    for (i = 0; i < npages; i++) {
        struct page* p = pd_request_page(&as->page_directory, data);
        p->F = 1; // Indicates that the page is on file
        data = data + PAGE_SIZE;
        p->PFN = (pindex << TEXT_SEGMENT_SHIFT) + i; 
    }
    
    as->as_data = vaddr;
    as->as_heap_start = data;
    as->as_heap_end = data;
    
    return 0;
}

int
as_prepare_load(struct addrspace *as) {

    DEBUG(DB_VM, "as_prepare_load\n");

    // Setup the USER STACK pages
    struct page* p = pd_request_page(&as->page_directory, USERSTACK - PAGE_SIZE);
    as->as_stacklocation = USERSTACK - PAGE_SIZE;

//    pd_print(&as->page_directory);

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
    
    // Temporarily use the exact same address space
    // TODO: Implement Copy on Write
    memmove(newas, old, sizeof (struct addrspace));
    
    // New address space must increase VOP_OPEN
    vfs_open(&old->progname, O_RDONLY, &newas->progfile);
    
    // Need to mark all physical memory with 2 so we don't have delete until all processes have ended
    
    *ret = newas;
    return 0;
}

void as_print(struct addrspace *as) {
    pd_print(&as->page_directory);
}
