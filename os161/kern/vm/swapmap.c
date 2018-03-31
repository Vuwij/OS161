#include <types.h>
#include <lib.h>
#include <swapmap.h>
#include <uio.h>
#include <thread.h>
#include <curthread.h>
#include <kern/unistd.h>
#include <vfs.h>
#include <vnode.h>
#include <kern/stat.h>
#include <addrspace.h>

int sm_pagecount;
struct vnode *swap_fp;
Swapmap* swapmap;
// Initializes the swap area using the disk

void sm_bootstrap() {

    // Open the Disk
    char sfname[] = SWAP_FILE;
    int result = vfs_open(sfname, O_RDWR, &swap_fp);
    if (result)
        panic("%d, VM: Failed to create Swap area\n", result);

    // Get information about the disk
    struct stat s;
    VOP_STAT(swap_fp, &s);
    sm_pagecount = s.st_size / PAGE_SIZE;
    kprintf("Opened Disk: %s\tSize: 0x%x\tBlocks 0x%x\tPages 0x%x", sfname, s.st_size, s.st_blocks, sm_pagecount);

    // bitmap
    swapmap = bitmap_create(sm_pagecount);
}

void sm_print() {
    kprintf("SWAP MAP\n");
    int i;
    for (i = 0; i < sm_pagecount; ++i) {
        if (bitmap_isset(swapmap, i))
            kprintf("+");
        else
            kprintf(".");
        if (i % 80 == 79)
            kprintf("\n");
    }
    kprintf("\n");
}

int sm_swapalloc(struct page* p) {
    // The page must be valid and have no page frame number
    assert(p->V);
    assert(p->PFN == 0);
    
    // Find a swap location using the swap map that can hold the page
    int pos;
    for (pos = 0; pos < sm_pagecount; ++pos) {
        if (!bitmap_isset(swapmap, pos)) {
            break;
        }
    }
    
    // Updates the bitmap to indicate that swap area is now used
    bitmap_mark(swapmap, pos);
    
    // Updates the page to point to the address in the swap file
    // Sets valid to false so OS will know it is disk address
    // PFN cannot be 0 because zero is used for unallocated memory
    p->PFN = pos + 1;
    p->V = 0;
    
    return 0;
}

int sm_swapdealloc(struct page* p) {
    // The page must be valid and have no page frame number
    assert(p->V == 0);
    assert(p->PFN != 0);
    assert(bitmap_isset(swapmap, p->PFN - 1))
    
    // Updates the bitmap to indicate that swap area is now used
    bitmap_unmark(swapmap, p->PFN - 1);
    
    p->PFN = 0;
    
    return 0;
}

int sm_swapout(struct page* p) {
    // The page must be valid to be swapped out
    assert(p->V);
    assert(p->PFN != 0);

    // Find a swap location using the swap map that can hold the page
    int pos;
    for (pos = 0; pos < sm_pagecount; ++pos) {
        if (!bitmap_isset(swapmap, pos)) {
            break;
        }
    }

    // Create a kernel UIO to prepare to write
    struct uio ku;
    mk_kuio(&ku, (void *) PADDR_TO_KVADDR((p->PFN << 12)), PAGE_SIZE, pos * PAGE_SIZE, UIO_WRITE);
    
    // Writes the swapped page into the disk
    int result = VOP_WRITE(swap_fp, &ku);
    if (result) return result;
    
    // Updates the bitmap to indicate that swap area is now used
    bitmap_mark(swapmap, pos);
    
    // Deallocates the page from memory. Page will automatically be invalidated upon free
    p_free_frame(p);
    
    // Updates the page to point to the address in the swap file
    // Sets valid to false so OS will know it is disk address
    // PFN cannot be 0 becuase zero is used for unallocated memory
    p->PFN = pos + 1;
    
    return 0;
}

int sm_swapin(struct page* p, vaddr_t vaddr) {
    // The page must be invalid to be swapped in
    assert(p->V == 0);
    int pos = p->PFN - 1;
    
    // Allocates a space on memory for the swapped in area
    paddr_t paddr = alloc_upages(1, vaddr);
    
    // Create a kernel UIO to prepare to read the location from the page frame number to memory
    struct uio ku;
    mk_kuio(&ku, (void *) PADDR_TO_KVADDR(paddr), PAGE_SIZE, pos * PAGE_SIZE, UIO_READ);
    
    // Reads from the UIO into the memory specified by paddr
    int result = VOP_READ(swap_fp, &ku);
    if (result) return result;
    
    // Updates the bitmap to indicate the swap area is now freed
    bitmap_unmark(swapmap, pos);
    
    // Updates the page to now point to the physical memory and revalidates the page
    p->PFN = (paddr >> 12);
    p->V = 1;
    
    return 0;
}
