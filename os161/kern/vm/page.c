#include <page.h>
#include <addrspace.h>
#include <swapmap.h>

void p_allocate_page(struct page* p) {
    
    // Allocates a disk location for the page
    if (p->PFN == 0 && p->V == 1) {
        sm_swapalloc(p);
    }
}

void p_print(struct page* p, int table, int page) {
    kprintf("%03x %03x %03x\t%d %d %d %d %d\n", table, page, p->PFN, p->M, p->R, p->V, p->F, p->Prot);
}

void p_copy(struct page* to, struct page* from, int table, int page) {    
    // If its in the memory, increment memory count
    if(from->V) {
        if (from->PFN != 0 && from->V == 1) {
            increment_frame(from->PFN);
        }
    }
    // Already loaded, in disk
    else if(!from->F) {
        sm_swapincrement(from);
    }
    
    memcpy(to, from, sizeof(struct page));
}

void p_free(struct page* p, int table, int page) {
    // If its in the memory, free memory
    if(p->V) {
        vaddr_t vaddr = (vaddr_t) ((table << 22) + (page << 12));

        if (p->PFN != 0 && p->V == 1) {
            free_upages(vaddr);
        }
    }
    // If its in the disk, remove bits from bitmap
    else {
        sm_swapdealloc(p);
    }
}

void p_free_frame(struct page* p) {
    if (p->PFN != 0 && p->V == 1) {
        free_frame(p->PFN);
        p->V = 0;
    }
}
