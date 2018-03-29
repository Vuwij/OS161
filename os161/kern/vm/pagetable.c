#include <pagetable.h>

#define PAGE_MASK 0x003FF000

paddr_t pt_request_page(struct pagetable* pt, vaddr_t vaddr) {
    unsigned entry = (((unsigned) vaddr) & PAGE_MASK) >> 12;
    
    return p_request_page(&pt->pte[entry], vaddr);
}
