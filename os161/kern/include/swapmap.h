#ifndef SWAPAREA_H
#define SWAPAREA_H

#include <types.h>
#include <lib.h>
#include <coremap.h>
#include <vm.h>
#include <bitmap.h>

#include "page.h"

#define SWAP_FILE "lhd0raw:" // Translates to disk 0

typedef struct bitmap {} Swapmap;

extern Swapmap* swapmap;

extern int sm_pagecount; // 1280 or 0x500

void sm_bootstrap();

void sm_print();

// Allocates space to swap memory (doesn't actually swap the memory)
int sm_swapalloc(struct page* p);

// Deallocates space to swap memory (when free is called)
int sm_swapdealloc(struct page* p);

// Finds a space in the swap file to swap out a piece of memory
int sm_swapout(struct page* p);

// Finds the spot in the swap file to swap in a piece of memory
int sm_swapin(struct page* p, vaddr_t vaddr);

#endif /* SWAPAREA_H */

