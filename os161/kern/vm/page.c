#include <page.h>
#include <addrspace.h>

void p_allocate_page(struct page* p, int table, int page) {

    // Creating the virtual address from the page and the table
    vaddr_t vaddr = (vaddr_t) ((table << 22) + (page << 12));
    
    if (p->PFN == 0) {
        paddr_t paddr = alloc_upages(1, vaddr);
        p->PFN = paddr >> 12;
        p->V = 1;
        return paddr;
    }
}

void p_print(struct page* p, int table, int page) {
    kprintf("%03x %03x %03x\t%d %d %d %d\n", table, page, p->PFN, p->M, p->R, p->V, p->Prot);
}
