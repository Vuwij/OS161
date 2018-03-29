#ifndef PAGEDIRECTORY_H
#define PAGEDIRECTORY_H

#include <types.h>
#include <lib.h>
#include <pagetable.h>

struct pagedirectory {
    struct pagetable* pde[1024]; // The page table is created on request
};

paddr_t pd_request_page(struct pagedirectory*, vaddr_t);

#endif /* PAGEDIRECTORY_H */

