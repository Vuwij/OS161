#ifndef PAGETABLE_H
#define PAGETABLE_H

#include <types.h>
#include <lib.h>
#include <page.h>

struct pagetable {
    struct page pte[1024];  // 2^10 page table entries per page table
};

struct page* pt_request_page(struct pagetable*, vaddr_t);

void pt_print(struct pagetable*, int table);

void pt_free(struct pagetable* pt, int table);

#endif /* PAGETABLE_H */

