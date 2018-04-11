#include <pagedirectory.h>

#include "lib.h"
#include "curthread.h"
#include "thread.h"
#include "coremap.h"

void pd_initialize(struct pagedirectory* pd) {
    int i;
    for (i = 0; i < 1024; ++i) {
        pd->pde[i] = NULL;
    }
}

struct page* pd_page_exists(struct pagedirectory* pd, vaddr_t vaddr) {
    unsigned entry = vaddr >> 22;
    if (pd->pde[entry] == NULL) {
        return NULL;
    }
    return pt_request_page(pd->pde[entry], vaddr);
}

struct page* pd_request_page(struct pagedirectory* pd, vaddr_t vaddr) {
    unsigned entry = vaddr >> 22;
    if (pd->pde[entry] == NULL) {
//        kprintf("Request Page\n");
        pd->pde[entry] = kmalloc(sizeof (struct pagetable));
        bzero(pd->pde[entry], sizeof (struct pagetable));
        paddr_t paddr = (((unsigned) pd->pde[entry]) & 0x00FFFFFF);
        struct coremap_entry* cmentry = cm_getcmentryfromaddress(paddr);
        cmentry->usecount = 1;
    }
    return pt_request_page(pd->pde[entry], vaddr);
}

void pd_print(struct pagedirectory* pd) {
    kprintf("Page Directory 0x%x for PID %d\n", pd, curthread->pid);
    kprintf("  Tab Pag PFN\tM R V F P\t\n");
    int i;
    for (i = 0; i < 1024; ++i) {
        if (pd->pde[i] != NULL) {
            kprintf("0x%x\n", pd->pde[i]);
            pt_print(pd->pde[i], i);
        }
    }
}

void pd_copy(struct pagedirectory* to, struct pagedirectory* from) {

    int i;
    for (i = 0; i < 1024; ++i) {
        if (from->pde[i] != NULL) {
            to->pde[i] = from->pde[i];
            
            paddr_t paddr = (((unsigned) from->pde[i]) & 0x00FFFFFF);
            struct coremap_entry* cmentry = cm_getcmentryfromaddress(paddr);
            cmentry->usecount++;
            
//            to->pde[i] = kmalloc(sizeof (struct pagetable));
//            bzero(to->pde[i], sizeof (struct pagetable));

            pt_copy(to->pde[i], from->pde[i], i);
        }
        else {
            to->pde[i] = NULL;
        }
    }
}

void pd_copy_on_write(struct pagedirectory* pd, vaddr_t vaddr) {
    unsigned entry = vaddr >> 22;
        
    struct pagetable* old = pd->pde[entry];
    pd->pde[entry] = kmalloc(sizeof (struct pagetable));
    memcpy(pd->pde[entry], old, sizeof (struct pagetable));
    
    paddr_t paddr = (((unsigned) old) & 0x00FFFFFF);
    struct coremap_entry* cmentry = cm_getcmentryfromaddress(paddr);
    cmentry->usecount--;
    
    paddr = (((unsigned) pd->pde[entry]) & 0x00FFFFFF);
    cmentry = cm_getcmentryfromaddress(paddr);
    cmentry->usecount = 1;
    
}

void pd_free(struct pagedirectory* pd) {
    int i;
    for (i = 0; i < 1024; ++i) {
        if (pd->pde[i] != NULL) {
            pt_free(pd->pde[i], i);
            
            // Check if its using someone elses page table
            paddr_t paddr = (((unsigned) pd->pde[i]) & 0x00FFFFFF);
            struct coremap_entry* cmentry = cm_getcmentryfromaddress(paddr);
            
            if(cmentry->usecount == 1) {
                kfree(pd->pde[i]);
                pd->pde[i] = NULL;
            }
            else if (cmentry->usecount > 1) {
                cmentry->usecount--;
            }
        }
    }
}

#define PAGE_MASK 0x003FF000
void pd_translate(vaddr_t vaddr) {
    unsigned table = vaddr >> 22;
    unsigned page = (((unsigned) vaddr) & PAGE_MASK) >> 12;
//    kprintf("Tab Pag PFN\n");
    kprintf("%03x %03x\n", table, page);
}
