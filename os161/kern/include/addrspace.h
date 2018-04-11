#ifndef _ADDRSPACE_H_
#define _ADDRSPACE_H_

#include <vm.h>
#include "opt-dumbvm.h"
#include <pagedirectory.h>
#include <uio.h>
#include <elf.h>

struct vnode;

/* 
 * Address space - data structure associated with the virtual memory
 * space of a process.
 *
 * You write this.
 */

struct addrspace {
    struct pagedirectory page_directory;
    
    char progname[20];
    struct vnode *progfile;
    Elf_Ehdr eh;

    vaddr_t as_codestart;
    vaddr_t as_codeend;
    vaddr_t as_stacklocation;
    vaddr_t as_heap_start;
    vaddr_t as_heap_end;
    vaddr_t as_data;
    
    struct lock* pdlock;
    struct node* lruclock; // Local replacement only
    struct node* lruhandle;
};

/*
 * Functions in addrspace.c:
 *
 *    as_create - create a new empty address space. You need to make 
 *                sure this gets called in all the right places. You
 *                may find you want to change the argument list. May
 *                return NULL on out-of-memory error.
 *
 *    as_copy   - create a new address space that is an exact copy of
 *                an old one. Probably calls as_create to get a new
 *                empty address space and fill it in, but that's up to
 *                you.
 *
 *    as_activate - make the specified address space the one currently
 *                "seen" by the processor. Argument might be NULL, 
 *		  meaning "no particular address space".
 *
 *    as_destroy - dispose of an address space. You may need to change
 *                the way this works if implementing user-level threads.
 *
 *    as_define_region - set up a region of memory within the address
 *                space.
 *
 *    as_prepare_load - this is called before actually loading from an
 *                executable into the address space.
 *
 *    as_complete_load - this is called when loading from an executable
 *                is complete.
 *
 *    as_define_stack - set up the stack region in the address space.
 *                (Normally called *after* as_complete_load().) Hands
 *                back the initial stack pointer for the new process.
 */

// Page Allocation and freeing for user and 
paddr_t alloc_upages(int npages, vaddr_t vaddr);
void zero_upages(vaddr_t addr);
void free_frame(int frame);
void increment_frame(int frame);

vaddr_t alloc_kpages(int npages);
void free_kpages(vaddr_t addr);

struct addrspace *as_create(void);
int               as_copy(struct addrspace *src, struct addrspace **ret);
void              as_activate(struct addrspace *);
void              as_destroy(struct addrspace *);
void              as_reset(struct addrspace *);
void              as_print(struct addrspace *);

int               as_define_region(struct addrspace *as, 
				   vaddr_t vaddr, int pindex, size_t sz,
				   int readable, 
				   int writeable,
				   int executable);
int		  as_prepare_load(struct addrspace *as);
int		  as_complete_load(struct addrspace *as);
int               as_define_stack(struct addrspace *as, vaddr_t *initstackptr);

/*
 * Functions in loadelf.c
 *    load_elf - load an ELF user program executable into the current
 *               address space. Returns the entry point (initial PC)
 *               in the space pointed to by ENTRYPOINT.
 *   
 *    load_elf_segment - called from vm_fault. Actually load the segment when demanded by the code
 */

int load_elf(struct vnode *v, vaddr_t *entrypoint);

int load_elf_segment(int segment, int part);

extern struct lock* copy_on_write_lock;

#define PRINT_RESET   "\033[0m"
#define PRINT_BLACK   "\033[30m"      /* Black */
#define PRINT_RED     "\033[31m"      /* Red */
#define PRINT_GREEN   "\033[32m"      /* Green */

#endif /* _ADDRSPACE_H_ */
