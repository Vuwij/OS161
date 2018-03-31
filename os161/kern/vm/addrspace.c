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


#define DUMBVM_STACKPAGES 12

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
    
    if (p->V && p->PFN) { // In memory
        paddr = (p->PFN << 12);
    }
    else if(p->V && p->PFN == 0) { // Requested not allocated
        kprintf("As expected\n");
        splx(spl);
        return EFAULT;
    }
    else if(!p->V && p->PFN) { // In disk
        sm_swapin(p, faultaddress);
        paddr = (p->PFN << 12);
    }
    else { 
        assert(faultaddress < USERSTACK);
        
        // Fault location is 
        if(faultaddress == as->as_stacklocation - PAGE_SIZE) {
            struct page* p = pd_request_page(&as->page_directory, faultaddress);    // Create a new page
            
            if(p->PFN != 0) return EFAULT;                                          // Page has hit the bottom
            
            p->V = 1;                                                               
            pd_allocate_pages(&as->page_directory);                                 // Allocate disk space for page
            sm_swapin(p, faultaddress);                                             // Swap the page from the disk
            paddr = (p->PFN << 12);
            as->as_stacklocation = as->as_stacklocation - PAGE_SIZE;                // Shrink the stack location, stack is never freed
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
        DEBUG(DB_VM, "  smartvm: 0x%x -> 0x%x\n", faultaddress, paddr); // Prints all the mapping in the TLB
        TLB_Write(ehi, elo, i);
        TLB_Read(&ehi, &elo, i);
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
    pd_initialize(&as->page_directory);
    if (as == NULL) {
        return NULL;
    }

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
    
//    pd_print(&as->page_directory);
//    cm_print();
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
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz,
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
    unsigned i;
    for (i = 0; i < npages; i++) {
        struct page* p = pd_request_page(&as->page_directory, vaddr);
        p->V = 1; // Temporarily set the valid bit to 1
        vaddr = vaddr + PAGE_SIZE;
    }
    
    return 0;
}

int
as_prepare_load(struct addrspace *as) {

    DEBUG(DB_VM, "as_prepare_load\n");
    
    // Setup the USER STACK pages
    struct page* p = pd_request_page(&as->page_directory, USERSTACK - PAGE_SIZE);
    p->V = 1;
    as->as_stacklocation = USERSTACK - PAGE_SIZE;
    
    // Automatically allocate all the pages without a page frame number. Allocate disk space for these pages
    pd_allocate_pages(&as->page_directory);
    
//    pd_print(&as->page_directory);
//    cm_print();
    
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
    memmove(newas, old, sizeof(struct addrspace));

    *ret = newas;
    return 0;
}

void as_print(struct addrspace *as) {
    pd_print(&as->page_directory);
}
