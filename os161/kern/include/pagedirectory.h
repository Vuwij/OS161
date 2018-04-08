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

// Same as pd_request_page, but doesn't create a page table if no exists. Returns NULL instead
struct page* pd_page_exists(struct pagedirectory*, vaddr_t);

// Requests for a page. if the page table doesn't exist, create the page table
struct page* pd_request_page(struct pagedirectory*, vaddr_t);

// Print the page directory
void pd_print(struct pagedirectory*);

// Make a copy of the page directory
void pd_copy(struct pagedirectory* to, struct pagedirectory* from);

// Free all the pages in the page directory
void pd_free(struct pagedirectory*);

// Translates the address
void pd_translate(vaddr_t);

#endif /* PAGEDIRECTORY_H */

