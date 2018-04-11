#include <types.h>
#include <lib.h>
#include <coremap.h>
#include <vm.h>

struct coremap_entry *coremap = NULL;
unsigned cm_totalframes, cm_totalkernelframes;
paddr_t firstpaddr, lastpaddr; // First physical address, used as offset

void coremap_bootstrap() {
    ram_getsize((u_int32_t *) &firstpaddr, (u_int32_t *) &lastpaddr);
    
    cm_totalframes = (lastpaddr - firstpaddr) / PAGE_SIZE;
    coremap = kmalloc(cm_totalframes * sizeof(struct coremap_entry));
    
    unsigned i;
    u_int32_t addr = firstpaddr;
    for (i = 0; i < cm_totalframes; i++) {
        coremap[i].addr = addr;
        addr = addr + PAGE_SIZE;
        coremap[i].usedby = CM_FREE;
        coremap[i].pid = 0;
        coremap[i].vaddr = 0; // Virtual Address is 0;
        coremap[i].length = 0;
    }
    
    // Get Coremap usage
    u_int32_t lo, hi;
    ram_getsize(&lo, &hi);
    
    int spaceleft = (hi - lo) / PAGE_SIZE;
    
    for (i = 0; i < cm_totalframes - spaceleft; i++) {
        coremap[i].usedby = CM_COREMAP;
    }
}

void coremap_getkernelusage() {
    // Need to include the Coremap in the Core Map
    unsigned i;
    unsigned kernelcount = 0;
    for (i = 0; i < cm_totalframes; i++) {
        if(coremap[i].usedby == CM_KTEMP) {
            coremap[i].usedby = CM_KERNEL;
            kernelcount++;
        }   
    }
    
    cm_totalkernelframes = kernelcount;
    
    cm_print();
}

unsigned cm_getframefromaddress(paddr_t paddr) {
    assert(paddr <= lastpaddr);
    if(paddr <= firstpaddr) {
        kprintf("Address requested invalid\n");
        kprintf("0x%x - 0x%x", paddr, firstpaddr);
    }
    assert(paddr > firstpaddr);
    return (paddr - firstpaddr) >> 12;
}

struct coremap_entry* cm_getcmentryfromaddress(paddr_t paddr) {
    assert(paddr <= lastpaddr);
    if(paddr <= firstpaddr) {
        kprintf("Address requested invalid\n");
        kprintf("0x%x - 0x%x", paddr, firstpaddr);
    }
    assert(paddr > firstpaddr);
    int id = (paddr - firstpaddr) >> 12;
    return &coremap[id];
}

void cm_print() {
    unsigned i;
    kprintf("\nFrame #\tPHY ADDR\tLength\tCount\tUSER\n");
    for (i = 0; i < cm_totalframes; i++) {
        if (coremap[i].usedby == CM_FREE) continue;
        
        kprintf("%d:\t0x%x\t", i, coremap[i].addr);
        kprintf("\t%d", coremap[i].length);
        kprintf("\t%d\t", coremap[i].usecount);
        
        if (coremap[i].usedby == CM_COREMAP)
            kprintf("COREMAP\n");
        if (coremap[i].usedby == CM_KERNEL)
            kprintf("KERNEL\n");
        if (coremap[i].usedby == CM_KTEMP)
            kprintf("KTEMP\n");
        if (coremap[i].usedby == CM_USED)
            kprintf("PID %d, VA: 0x%x\n", coremap[i].pid, coremap[i].vaddr);
    }
}
