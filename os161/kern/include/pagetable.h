#ifndef PAGETABLE_H
#define PAGETABLE_H

#include <types.h>
#include <lib.h>
#include <page.h>

struct pagetable {
    struct page pte[1024];  // 2^10 page table entries per page table
};

paddr_t pt_request_page(struct pagetable*, vaddr_t);

#endif /* PAGETABLE_H */

