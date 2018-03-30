#ifndef PAGEDIRECTORY_H
#define PAGEDIRECTORY_H

#include <types.h>
#include <lib.h>
#include <pagetable.h>
#include <page.h>

struct pagedirectory {
    struct pagetable* pde[1024]; // The page table is created on request
};

void pd_initialize(struct pagedirectory*);

struct page* pd_request_page(struct pagedirectory*, vaddr_t);

void pd_allocate_pages(struct pagedirectory*);

void pd_print(struct pagedirectory*);

void pd_free(struct pagedirectory*);

#endif /* PAGEDIRECTORY_H */

