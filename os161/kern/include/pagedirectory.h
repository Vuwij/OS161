#ifndef PAGEDIRECTORY_H
#define PAGEDIRECTORY_H

#include <types.h>
#include <lib.h>
#include <pagetable.h>
#include <page.h>

struct pagedirectory {
    struct pagetable* pde[1024]; // The page table is created on request
};

// Intializes the page directory
void pd_initialize(struct pagedirectory*);

// Requests for a page. if the page table doesn't exist, create the page table
struct page* pd_request_page(struct pagedirectory*, vaddr_t);

// Sets a physical memory for a page
void pd_allocate_pages(struct pagedirectory*);

// Print the page directory
void pd_print(struct pagedirectory*);

// Free all the pages in the page directory
void pd_free(struct pagedirectory*);

#endif /* PAGEDIRECTORY_H */

