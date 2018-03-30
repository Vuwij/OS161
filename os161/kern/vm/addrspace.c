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
    
    /* nothing */
    /*int index;
    u_int32_t ehi, elo;
    
    index = TLB_Probe(addr,0);
    if (index == -1) {
        
    }
    TLB_Read(&ehi,&elo,index);*/

    //(void) addr;
    //ram_returnmem(addr);
}

int
vm_fault(int faulttype, vaddr_t faultaddress) {

    vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop;
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
    
    // Page is valid and physical address is setup
    if(p->V == 1) {
        paddr = (p->PFN << 12);
    }
    else {
        // Temporarily invalid address requested
        splx(spl);
        return EFAULT;
    }

    // TLB Stuff
    assert((paddr & PAGE_FRAME) == paddr);
    for (i = 0; i < NUM_TLB; i++) {
        TLB_Read(&ehi, &elo, i);
        if (elo & TLBLO_VALID) {
            continue;
        }
        ehi = faultaddress;
        elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
        DEBUG(DB_VM, "  smartvm: 0x%x -> 0x%x\n", faultaddress, paddr);
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
    pd_free(&as->page_directory);
    kfree(as);
    as = NULL;
}

void
as_activate(struct addrspace *as) {
    int i, spl;

    (void) as;

    spl = splhigh();

    for (i = 0; i < NUM_TLB; i++) {
        TLB_Write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
    }
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
    int i;
    for (i = 0; i < npages; i++) {
        struct page* p = pd_request_page(&as->page_directory, vaddr);
        p->V = 1; // Temporarily set the valid bit to 1
        vaddr = vaddr + PAGE_SIZE;
    }
}

int
as_prepare_load(struct addrspace *as) {

    DEBUG(DB_VM, "as_prepare_load\n");
    
    // Setup the USER STACK pages
    struct page* p = pd_request_page(&as->page_directory, USERSTACK - PAGE_SIZE);
    p->V = 1;
    
    // Automatically allocate all the pages without a page frame number
    pd_allocate_pages(&as->page_directory);
    
    pd_print(&as->page_directory);
    
    cm_print();
    
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
