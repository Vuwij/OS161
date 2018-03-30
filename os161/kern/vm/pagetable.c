#include <pagetable.h>

#define PAGE_MASK 0x003FF000

struct page* pt_request_page(struct pagetable* pt, vaddr_t vaddr) {
    unsigned entry = (((unsigned) vaddr) & PAGE_MASK) >> 12;
    
    return &pt->pte[entry];
}

void pt_allocate_page(struct pagetable* pt, int table) {
    int i;
    for (i = 0; i < 1024; ++i) {
        if(pt->pte[i].V != 0) {
            p_allocate_page(&pt->pte[i], table, i);
        }
    }
}

void pt_print(struct pagetable* pt, int table) {
    int i;
    for (i = 0; i < 1024; ++i) {
        if(pt->pte[i].V != 0 || pt->pte[i].PFN != 0) {
            p_print(&pt->pte[i], table, i);
        }
    }
}

void pt_free(struct pagetable* pt, int table) {
    int i;
    for (i = 0; i < 1024; ++i) {
        if(pt->pte[i].V != 0 || pt->pte[i].PFN != 0) {
            p_free(&pt->pte[i], table, i);
        }
    }
}