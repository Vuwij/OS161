#include <pagedirectory.h>

#include "lib.h"
#include "curthread.h"
#include "thread.h"

void pd_initialize(struct pagedirectory* pd) {
    int i;
    for (i = 0; i < 1024; ++i) {
        pd->pde[i] = NULL;
    }
}

struct page* pd_page_exists(struct pagedirectory* pd, vaddr_t vaddr) {
    unsigned entry = vaddr >> 22;
    if (pd->pde[entry] == NULL) {
        return NULL;
    }
    return pt_request_page(pd->pde[entry], vaddr);
}

struct page* pd_request_page(struct pagedirectory* pd, vaddr_t vaddr) {
    unsigned entry = vaddr >> 22;
    if (pd->pde[entry] == NULL) {
        pd->pde[entry] = kmalloc(sizeof (struct pagetable));
        bzero(pd->pde[entry], sizeof (struct pagetable));
    }
    return pt_request_page(pd->pde[entry], vaddr);
}

void pd_print(struct pagedirectory* pd) {
    kprintf("Tab Pag PFN\tM R V F P\t\n");
    int i;
    for (i = 0; i < 1024; ++i) {
        if (pd->pde[i] != NULL) {
            pt_print(pd->pde[i], i);
        }
    }
}

void pd_copy(struct pagedirectory* to, struct pagedirectory* from) {

    int i;
    for (i = 0; i < 1024; ++i) {
        if (from->pde[i] != NULL) {
            to->pde[i] = kmalloc(sizeof (struct pagetable));
            bzero(to->pde[i], sizeof (struct pagetable));

            pt_copy(to->pde[i], from->pde[i], i);
        }
    }
}


void pd_free(struct pagedirectory* pd) {
    int i;
    for (i = 0; i < 1024; ++i) {
        if (pd->pde[i] != NULL) {
            pt_free(pd->pde[i], i);
            kfree(pd->pde[i]);
            pd->pde[i] = NULL;
        }
    }
}
