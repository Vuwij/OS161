#include <pagedirectory.h>

#include "lib.h"

paddr_t pd_request_page(struct pagedirectory* pd, vaddr_t vaddr) {
    unsigned entry = vaddr >> 22;
    if(pd->pde[entry] == NULL) {
        pd->pde[entry] = kmalloc(sizeof(struct pagetable));
        bzero(pd->pde[entry], sizeof(struct pagetable));
    }
    return pt_request_page(pd->pde[entry], vaddr);
}
