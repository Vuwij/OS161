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
#include <kern/errno.h>

#include "synch.h"

int sm_pagecount;
struct vnode *swap_fp;
Swapmap* swapmap;
struct lock* swapmaplock;
struct node* swapcount;

#define DEBUG_SWAP 0
#define DEBUG_SWAPMAP 0

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
    
    // Swapmap lock
    swapmaplock = lock_create("Swapmap Lock");
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

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */

void sm_print_debug() {
    kprintf(RED " SM PID %d " BLACK, curthread->pid);
    int i;
    for (i = 0; i < sm_pagecount; i = i + 2) {
        if (bitmap_isset(swapmap, i)) {
            struct node* n = swapexists(swapcount, i);
            if(n == NULL)
                kprintf("1");
            else
                kprintf("%d", SWAPCOUNT_COUNT(n->val));
        }
        else
            kprintf(".");
        if (i > 400)
            break;
    }
    kprintf("\n");
}

int sm_swapalloc(struct page* p) {
    // The page must be valid and have no page frame number
    assert(p->V);
    assert(p->PFN == 0);

    // Find a swap location using the swap map that can hold the page
    lock_acquire(swapmaplock);
    
    int pos;
    for (pos = 0; pos < sm_pagecount; ++pos) {
        if (!bitmap_isset(swapmap, pos)) {
            break;
        }
    }
    

    // Updates the page to point to the address in the swap file
    // Sets valid to false so OS will know it is disk address
    // PFN cannot be 0 because zero is used for unallocated memory
    p->PFN = pos + 1;

    // Updates the bitmap to indicate that swap area is now used
    sm_swapincrement(p);
    p->V = 0;
    
    lock_release(swapmaplock);
    
    return 0;
}

int sm_swapdealloc(struct page* p) {
    // The page must be valid and have no page frame number
    assert(p->V == 0);
    assert(p->PFN != 0);
//    kprintf("0x%x\n", p->PFN << 12);
    int pos = p->PFN - 1;
    
    lock_acquire(swapmaplock);
    
    assert(bitmap_isset(swapmap, pos))
    
    // Updates the bitmap to indicate the swap area is now freed
    sm_swapdecrement(p);
    
    lock_release(swapmaplock);
    
    p->PFN = 0;

    return 0;
}

int sm_swapout(struct page* p, vaddr_t vaddr) {
    // The page must be valid to be swapped out
    assert(p->V);
    assert(p->PFN != 0);

    // Find a swap location using the swap map that can hold the page
    lock_acquire(swapmaplock);
    
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
    
    // Deallocates the page from memory. Page will automatically be invalidated upon free
    p_free_frame(p);
    
    // Updates the page to point to the address in the swap file
    // Sets valid to false so OS will know it is disk address
    // PFN cannot be 0 becuase zero is used for unallocated memory
    p->PFN = pos + 1;

    // Updates the bitmap to indicate that swap area is now used
    sm_swapincrement(p);
    
    lock_release(swapmaplock);
    
    p->V = 0;
    incrementlruclock();
    remove_val(&curthread->t_vmspace->lruclock, vaddr);

    return 0;
}

int sm_swapin(struct page* p, vaddr_t vaddr) {
    // The page must be invalid to be swapped in
    assert(p->V == 0);
    int pos = p->PFN - 1;

    // Allocates a space on memory for the swapped in area
    paddr_t paddr = alloc_upages(1, vaddr);

    // Ran out of memory to swap in, swaps out something else from the local
    if (paddr == 0) {
        cm_print();
        while(1);
        
        vaddr_t swapoutaddr = findnextlruclockframe();
        assert(swapoutaddr != vaddr);
        struct page* p = pd_page_exists(&curthread->t_vmspace->page_directory, swapoutaddr);
        assert(p != NULL);
        assert(p->V);
        
        // Actually swap out that page
        kprintf("0x%x-> 0x%x\n", swapoutaddr, vaddr);
        cm_print();
        if(DEBUG_SWAP) {
            kprintf("------- Thread %d: Swapping 0x%x-> 0x%x -------\n", curthread->pid, swapoutaddr, vaddr);
            kprintf("Before\n");
            sm_print();
            cm_print();
        }
        sm_swapout(p, swapoutaddr);
        // Swap in the memory again
        paddr = alloc_upages(1, vaddr);
        
        if(DEBUG_SWAP) {
            kprintf("After\n");
            sm_print();
            cm_print();
            kprintf("-------------------------------------------------");
        }
        assert(paddr != 0);
    }

    // Create a kernel UIO to prepare to read the location from the page frame number to memory
    struct uio ku;
    mk_kuio(&ku, (void *) PADDR_TO_KVADDR(paddr), PAGE_SIZE, pos * PAGE_SIZE, UIO_READ);

    // Reads from the UIO into the memory specified by paddr
    int result = VOP_READ(swap_fp, &ku);
    if (result) return result;

    // Updates the bitmap to indicate the swap area is now freed
    lock_acquire(swapmaplock);
    sm_swapdecrement(p);
    lock_release(swapmaplock);
    
    // Updates the page to now point to the physical memory and revalidates the page
    p->PFN = (paddr >> 12);
    p->V = 1;
    p->R = 1; // Swapped in pages have reference = 1;

    // Add the page to the LRU clock
    push_end(&(curthread->t_vmspace->lruclock), vaddr);
//    print_list(curthread->t_vmspace->lruclock);
//    kprintf("\n");
    return 0;
}

int sm_swapdecrement(struct page* p) {
    assert(p->PFN != 0);
    int pos = p->PFN - 1;
    
    struct node* n = swapexists(swapcount, pos);
    
    // If marked only once, decrement marker
    if (n == NULL) {
        bitmap_unmark(swapmap, pos);
    }        // If marked once, remove the value and unmark
    else if (SWAPCOUNT_COUNT(n->val) == 1) {
        remove_val(&swapcount, n->val);
    }        // If marked twice, decrement swapmap count
    else {
        n->val--;
    }
    
    if (DEBUG_SWAPMAP) sm_print_debug();
    return 0;
}

int sm_swapincrement(struct page* p) {
    // Page must be invalid (located on disk)
    assert(p->PFN != 0);
    int pos = p->PFN - 1;
    
    int b = bitmap_isset(swapmap, pos);
    
    // If unmarked, simply mark it
    if (b == 0) {
        bitmap_mark(swapmap, pos);
        if (DEBUG_SWAPMAP) sm_print_debug();
        return 0;
    }
    
    // If already marked once, increment the marker in swapcount
    struct node* n = swapexists(swapcount, pos);
    if (n == NULL) {
        int entry = (pos << 16) + 1;
        push_end(&swapcount, entry);
    }        // If already marked twice, increment the marker value even more
    else {
        n->val++;
    }
    if (DEBUG_SWAPMAP) sm_print_debug();
    return 0;
}

// Advances the LRU clock handle by one
vaddr_t incrementlruclock() {
    if (curthread->t_vmspace->lruclock == 0)
        return 0;

    if (curthread->t_vmspace->lruhandle->next != 0)
        curthread->t_vmspace->lruhandle = curthread->t_vmspace->lruhandle->next;
    else
        curthread->t_vmspace->lruhandle = curthread->t_vmspace->lruclock; // Back to the start of the clock
    
    return (vaddr_t) curthread->t_vmspace->lruhandle->val;
}

// LRU clock frame algorithm
vaddr_t findnextlruclockframe() {
    assert(curthread->t_vmspace->lruclock != NULL); // Need to wait
    if(curthread->t_vmspace->lruhandle == NULL) curthread->t_vmspace->lruhandle = curthread->t_vmspace->lruclock;
    
    struct node* n = curthread->t_vmspace->lruhandle;
    unsigned vaddr = n->val;
    
    // Set current clock handle to 0
    struct page* p = pd_page_exists(&curthread->t_vmspace->page_directory, vaddr);
    assert(p->V);
    assert(p != NULL);
    p->R = 0;
    
    while(1) {
        unsigned nextaddr = incrementlruclock();
        struct page* p = pd_page_exists(&curthread->t_vmspace->page_directory, nextaddr);
        assert(p->V);
        assert(p != NULL);
        if(p->R == 1)
            p->R = 0;
        else {
            return nextaddr;
        }
    }
}
