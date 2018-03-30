#include <pagedirectory.h>

#include "lib.h"

struct page* pd_request_page(struct pagedirectory* pd, vaddr_t vaddr) {
    unsigned entry = vaddr >> 22;
    if(pd->pde[entry] == NULL) {
        pd->pde[entry] = kmalloc(sizeof(struct pagetable));
        bzero(pd->pde[entry], sizeof(struct pagetable));
    }
    return pt_request_page(pd->pde[entry], vaddr);
}

void pd_allocate_pages(struct pagedirectory* pd) {
    kprintf("Tab Pag PFN\tM R V P\t\n");
    int i;
    for (i = 0; i < 1024; ++i) {
        if(pd->pde[i] != NULL) {
            pt_allocate_page(pd->pde[i], i);
        }
    }
}

void pd_print(struct pagedirectory* pd) {
    kprintf("Tab Pag PFN\tM R V P\t\n");
    int i;
    for (i = 0; i < 1024; ++i) {
        if(pd->pde[i] != NULL) {
            pt_print(pd->pde[i], i);
        }
    }
}

void pd_free(struct pagedirectory* pd) {
    int i;
    for (i = 0; i < 1024; ++i) {
        if(pd->pde[i] != NULL) {
            pt_free(pd->pde[i], i);
            kfree(pd->pde[i]);
        }
    }
}