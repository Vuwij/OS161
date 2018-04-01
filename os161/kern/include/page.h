#ifndef PAGE_H
#define PAGE_H

#include <types.h>
#include <lib.h>

struct page {
    u_int32_t M     : 1;        // Modify Bit
    u_int32_t R     : 1;        // Reference Bit
    u_int32_t V     : 1;        // Valid Bit
    u_int32_t F     : 1;        // File Bit (Indicates page part of file)
    u_int32_t Prot  : 3;        // Protection Bit
    u_int32_t PFN   : 25;       // Page Frame Number (0 - 61 (0b111101))
};

void p_allocate_page(struct page*);

void p_print(struct page*, int table, int page);

// Free the page using the virtual address
void p_free(struct page* p, int table, int page);

// Free the page using the frame number
void p_free_frame(struct page* p);

#endif /* PAGE_H */

