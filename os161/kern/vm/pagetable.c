#include <pagetable.h>

#define PAGE_MASK 0x003FF000


struct page* pt_request_page(struct pagetable* pt, vaddr_t vaddr) {
    unsigned entry = (((unsigned) vaddr) & PAGE_MASK) >> 12;
    
    return &pt->pte[entry];
}

void pt_print(struct pagetable* pt, int table) {
    int i;
    for (i = 0; i < 1024; ++i) {
        if(pt->pte[i].V != 0 || pt->pte[i].PFN != 0 || pt->pte[i].F != 0) {
            p_print(&pt->pte[i], table, i);
        }
    }
}

void pt_copy(struct pagetable* to, struct pagetable* from, int table) {
    int i;
    for (i = 0; i < 1024; ++i) {
        // Must be in physical memory or disk
        if(from->pte[i].V != 0 || from->pte[i].PFN != 0) {
            p_copy(&to->pte[i], &from->pte[i], table, i);
        }
    }
}

void pt_free(struct pagetable* pt, int table) {
    int i;
    for (i = 0; i < 1024; ++i) {
        // Unread text segment
        if(pt->pte[i].V == 0 && pt->pte[i].F == 1) {
            continue;
        }
        
        // Must be in physical memory or disk
        if(pt->pte[i].V != 0 || pt->pte[i].PFN != 0) {
            p_free(&pt->pte[i], table, i);
        }
    }
}
