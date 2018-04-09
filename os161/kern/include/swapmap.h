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

#define SWAPCOUNT_COUNT(paddr) ((paddr) & 0x0000FFFF) // The swap file count
#define SWAPCOUNT_SWAP(paddr) ((paddr) >> 16)         // The swap file index

extern struct node* swapcount; // For situations where 2 processes using the same swap area

extern int sm_pagecount; // 1280 or 0x500

void sm_bootstrap();

void sm_print();

// Allocates space to swap memory (doesn't actually swap the memory)
int sm_swapalloc(struct page* p);

// Deallocates space to swap memory (when free is called)
int sm_swapdealloc(struct page* p);

// Finds a space in the swap file to swap out a piece of memory
int sm_swapout(struct page* p, vaddr_t vaddr);

// Finds the spot in the swap file to swap in a piece of memory
int sm_swapin(struct page* p, vaddr_t vaddr);

// Actually increment the swap bit. But uses the external linked list to count doubles
int sm_swapdecrement(struct page* p);

// Actually decrement the swap bit. But uses the external linked list to count doubles
int sm_swapincrement(struct page* p);

// Increment the LRU clock to the next one, returns vaddr
vaddr_t incrementlruclock();
vaddr_t findnextlruclockframe();

#endif /* SWAPAREA_H */

