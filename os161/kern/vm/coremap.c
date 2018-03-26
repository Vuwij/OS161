#include <types.h>
#include <lib.h>
#include <coremap.h>
#include <vm.h>

struct coremap_entry *coremap = NULL;
int cm_totalframes;

void coremap_bootstrap() {
    u_int32_t firstpaddr, lastpaddr;
    ram_getsize(&firstpaddr, &lastpaddr);
    
    cm_totalframes = (lastpaddr - firstpaddr) / PAGE_SIZE;
    coremap = kmalloc(cm_totalframes * sizeof(struct coremap_entry));
    
    int i;
    u_int32_t addr = firstpaddr;
    for (i = 0; i < cm_totalframes; i++) {
        coremap[i].addr = addr;
        addr = addr + PAGE_SIZE;
        coremap[i].usedby = CM_FREE;
        coremap[i].pid = 0;
        coremap[i].vaddr = 0; // Virtual Address is 0;
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
    u_int32_t lo, hi;
    ram_getsize(&lo, &hi);
    
    int spaceleft = (hi - lo) / PAGE_SIZE;
    
    int i;
    for (i = 0; i < cm_totalframes; i++) {
        if(coremap[i].usedby == CM_USED) // The used area is the kernel area
            coremap[i].usedby = CM_KERNEL;
    }
    cm_print();
}

void cm_print() {
    int i;
    kprintf("\nFrame #\tPHY ADDR\t USER\n");
    for (i = 0; i < cm_totalframes; i++) {
        kprintf("%d:\t0x%x\t\t", i, coremap[i].addr);
        if (coremap[i].usedby == CM_FREE)
            kprintf("FREE");
        if (coremap[i].usedby == CM_COREMAP)
            kprintf("COREMAP");
        if (coremap[i].usedby == CM_KERNEL)
            kprintf("KERNEL");
        if (coremap[i].usedby == CM_USED)
            kprintf("PID %d, VA: 0x%x", coremap[i].pid, coremap[i].vaddr);
        kprintf("\n");
    }
}
