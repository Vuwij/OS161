#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/spl.h>
#include <machine/tlb.h>

/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 */

#define DUMBVM_STACKPAGES    12


void
vm_bootstrap(void) {
    /* Do nothing. */
}

static
paddr_t
getppages(unsigned long npages) {
    DEBUG(DB_VM, "  getppages: %d\n", npages);
    
    int spl;
    paddr_t addr;

    spl = splhigh();

    addr = ram_stealmem(npages);

    splx(spl);
    return addr;
}

/* Allocate/free some kernel-space virtual pages */
vaddr_t
alloc_kpages(int npages) {
    DEBUG(DB_VM, "alloc_kpages: %d\n", npages);
    
    paddr_t pa;
    pa = getppages(npages);
    if (pa == 0) {
        return 0;
    }
    return PADDR_TO_KVADDR(pa);
}

void
free_kpages(vaddr_t addr) {
    DEBUG(DB_VM, "  free_kpages: 0x%x\n", addr);
    /* nothing */

    (void) addr;
}

int
vm_fault(int faulttype, vaddr_t faultaddress) {
//    DEBUG(DB_VM, "vm_fault: faulttype: %d, address 0x%x\n", faultaddress);
    
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
    if (as == NULL) {
        /*
         * No address space set up. This is probably a kernel
         * fault early in boot. Return EFAULT so as to panic
         * instead of getting into an infinite faulting loop.
         */
        return EFAULT;
    }

    /* Assert that the address space has been set up properly. */
    assert(as->as_vbase1 != 0);
    assert(as->as_pbase1 != 0);
    assert(as->as_npages1 != 0);
    assert(as->as_vbase2 != 0);
    assert(as->as_pbase2 != 0);
    assert(as->as_npages2 != 0);
    assert(as->as_stackpbase != 0);
    assert((as->as_vbase1 & PAGE_FRAME) == as->as_vbase1);
    assert((as->as_pbase1 & PAGE_FRAME) == as->as_pbase1);
    assert((as->as_vbase2 & PAGE_FRAME) == as->as_vbase2);
    assert((as->as_pbase2 & PAGE_FRAME) == as->as_pbase2);
    assert((as->as_stackpbase & PAGE_FRAME) == as->as_stackpbase);

    vbase1 = as->as_vbase1;
    vtop1 = vbase1 + as->as_npages1 * PAGE_SIZE;
    vbase2 = as->as_vbase2;
    vtop2 = vbase2 + as->as_npages2 * PAGE_SIZE;
    stackbase = USERSTACK - DUMBVM_STACKPAGES * PAGE_SIZE;
    stacktop = USERSTACK;

    if (faultaddress >= vbase1 && faultaddress < vtop1) {
        paddr = (faultaddress - vbase1) + as->as_pbase1;
    } else if (faultaddress >= vbase2 && faultaddress < vtop2) {
        paddr = (faultaddress - vbase2) + as->as_pbase2;
    } else if (faultaddress >= stackbase && faultaddress < stacktop) {
        paddr = (faultaddress - stackbase) + as->as_stackpbase;
    } else {
        splx(spl);
        return EFAULT;
    }

    /* make sure it's page-aligned */
    assert((paddr & PAGE_FRAME) == paddr);

    for (i = 0; i < NUM_TLB; i++) {
        TLB_Read(&ehi, &elo, i);
        if (elo & TLBLO_VALID) {
            continue;
        }
        ehi = faultaddress;
        elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
        DEBUG(DB_VM, "  dumbvm: 0x%x -> 0x%x\n", faultaddress, paddr);
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

    /*
     * Write this.
     */

    as->as_vbase1 = 0;
    as->as_pbase1 = 0;
    as->as_npages1 = 0;
    as->as_vbase2 = 0;
    as->as_pbase2 = 0;
    as->as_npages2 = 0;
    as->as_stackpbase = 0;

    return as;
}

void
as_reset(struct addrspace *as) {
    DEBUG(DB_VM, "as_reset\n");
    if (as == NULL) {
        return;
    }

    as->as_vbase1 = 0;
    as->as_pbase1 = 0;
    as->as_npages1 = 0;
    as->as_vbase2 = 0;
    as->as_pbase2 = 0;
    as->as_npages2 = 0;
    as->as_stackpbase = 0;
}

void
as_destroy(struct addrspace *as) {
    DEBUG(DB_VM, "as_destroy\n");

    kfree(as);
}

void
as_activate(struct addrspace *as) {
//    DEBUG(DB_VM, "as_activate\n");
    
    /*
     * Write this.
     */
    int i, spl;

    (void) as;

    spl = splhigh();

    for (i = 0; i < NUM_TLB; i++) {
        TLB_Write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
    }

    (void) as; // suppress warning until code gets written
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

    /* We don't use these - all pages are read-write */
    (void) readable;
    (void) writeable;
    (void) executable;

    if (as->as_vbase1 == 0) {
        as->as_vbase1 = vaddr;
        as->as_npages1 = npages;
        return 0;
    }

    if (as->as_vbase2 == 0) {
        as->as_vbase2 = vaddr;
        as->as_npages2 = npages;
        return 0;
    }

    /*
     * Support for more than two regions is not available.
     */
    kprintf("dumbvm: Warning: too many regions\n");
    return EUNIMP;
}

int
as_prepare_load(struct addrspace *as) {
    /*
     * Write this.
     */

    DEBUG(DB_VM, "as_prepare_load\n");
    
    assert(as->as_pbase1 == 0);
    assert(as->as_pbase2 == 0);
    assert(as->as_stackpbase == 0);

    as->as_pbase1 = getppages(as->as_npages1);
    if (as->as_pbase1 == 0) {
        return ENOMEM;
    }

    as->as_pbase2 = getppages(as->as_npages2);
    if (as->as_pbase2 == 0) {
        return ENOMEM;
    }

    as->as_stackpbase = getppages(DUMBVM_STACKPAGES);
    if (as->as_stackpbase == 0) {
        return ENOMEM;
    }


    (void) as;
    return 0;
}

int
as_complete_load(struct addrspace *as) {
    /*
     * Write this.
     */
    DEBUG(DB_VM, "as_complete_load\n");
    
    (void) as;
    return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr) {
    /*
     * Write this.
     */
    DEBUG(DB_VM, "as_define_stack\n");
    
    (void) as;

    assert(as->as_stackpbase != 0);

    /* Initial user-level stack pointer */
    *stackptr = USERSTACK;

    return 0;
}

int
as_copy(struct addrspace *old, struct addrspace **ret) {
    /*
     * Write this.
     */
    DEBUG(DB_VM, "as_copy\n");
    
    struct addrspace *newas;

    newas = as_create();
    if (newas == NULL) {
        return ENOMEM;
    }

    newas->as_vbase1 = old->as_vbase1;
    newas->as_npages1 = old->as_npages1;
    newas->as_vbase2 = old->as_vbase2;
    newas->as_npages2 = old->as_npages2;

    if (as_prepare_load(newas)) {
        as_destroy(newas);
        return ENOMEM;
    }

    assert(newas->as_pbase1 != 0);
    assert(newas->as_pbase2 != 0);
    assert(newas->as_stackpbase != 0);

    memmove((void *) PADDR_TO_KVADDR(newas->as_pbase1),
            (const void *) PADDR_TO_KVADDR(old->as_pbase1),
            old->as_npages1 * PAGE_SIZE);

    memmove((void *) PADDR_TO_KVADDR(newas->as_pbase2),
            (const void *) PADDR_TO_KVADDR(old->as_pbase2),
            old->as_npages2 * PAGE_SIZE);

    memmove((void *) PADDR_TO_KVADDR(newas->as_stackpbase),
            (const void *) PADDR_TO_KVADDR(old->as_stackpbase),
            DUMBVM_STACKPAGES * PAGE_SIZE);

    (void) old;

    *ret = newas;
    return 0;
}