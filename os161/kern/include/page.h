#ifndef PAGE_H
#define PAGE_H

#include <types.h>
#include <lib.h>

struct page {
    u_int32_t M     : 1;        // Modify Bit
    u_int32_t R     : 1;        // Reference Bit
    u_int32_t V     : 1;        // Valid Bit
    u_int32_t Prot  : 2;        // Protection Bit
    u_int32_t PFN   : 27;       // Page Frame Number (0 - 61 (0b111101))
};

paddr_t p_request_page(struct page*, vaddr_t);


#endif /* PAGE_H */

