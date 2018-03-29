#include <page.h>
#include <addrspace.h>

paddr_t p_request_page(struct page* p, vaddr_t vaddr) {
    if(p->V == 0) {
        paddr_t paddr = getppages(1);
        p->PFN = paddr >> 12;
        p->V = 1;
        return paddr;
    }
    else {
        return p->PFN << 12;
    }
}

