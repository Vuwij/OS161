#include <types.h>
#include <lib.h>
#include <vm.h>
#include <machine/pcb.h>
#include <coremap.h>

u_int32_t firstfree; /* first free virtual address; set by start.S */

static u_int32_t firstpaddr; /* address of first free physical page */
static u_int32_t lastpaddr; /* one past end of last free physical page */

/*
 * Called very early in system boot to figure out how much physical
 * RAM is available.
 */
void
ram_bootstrap(void) {
    u_int32_t ramsize;

    /* Get size of RAM. */
    ramsize = mips_ramsize();

    /*
     * This is the same as the last physical address, as long as
     * we have less than 508 megabytes of memory. If we had more,
     * various annoying properties of the MIPS architecture would
     * force the RAM to be discontiguous. This is not a case we 
     * are going to worry about.
     */
    if (ramsize > 508 * 1024 * 1024) {
        ramsize = 508 * 1024 * 1024;
    }

    lastpaddr = ramsize;

    /* 
     * Get first free virtual address from where start.S saved it.
     * Convert to physical address.
     */
    firstpaddr = firstfree - MIPS_KSEG0;

    kprintf("Cpu is MIPS r2000/r3000\n");
    kprintf("%uk physical memory available\n",
            (lastpaddr - firstpaddr) / 1024);
    kprintf("%u total physical pages available\n",
            (lastpaddr - firstpaddr) / PAGE_SIZE);
}

/*
 * This function is for allocating physical memory prior to VM
 * initialization.
 *
 * The pages it hands back will not be reported to the VM system when
 * the VM system calls ram_getsize(). If it's desired to free up these
 * pages later on after bootup is complete, some mechanism for adding
 * them to the VM system's page management must be implemented.
 *
 * Note: while the error return value of 0 is a legal physical address,
 * it's not a legal *allocatable* physical address, because it's the
 * page with the exception handlers on it.
 *
 * This function should not be called once the VM system is initialized, 
 * so it is not synchronized.
 */
paddr_t
ram_stealmem(unsigned long npages) {
    u_int32_t size = npages * PAGE_SIZE;
    u_int32_t paddr;
    kprintf("lastpaddr 0x%x", lastpaddr);
    if (firstpaddr + size > lastpaddr) {
        return 0;
    }

    paddr = firstpaddr;
    firstpaddr += size;

    return paddr;
}

// Borrow some memory as kernel
paddr_t
ram_borrowmem(unsigned long npages) {
    if(coremap == NULL)
        return ram_stealmem(npages);
    
    
    int count = 0, i, startframe = -1;
    for(i = 0; i < cm_totalframes; ++i) {
        if(coremap[i].usedby == CM_FREE) {
            count++;
            if(count == npages) {
                startframe = i - npages + 1;
            }
        }
    }
    
    if(startframe == -1) return 0;
    
    for (i = startframe; i < startframe + npages; ++i) {
        coremap[i].usedby = CM_KTEMP;
    }
    
    // Set the length of the mapped memory (so it can be deleted)
    coremap[startframe].length = npages;
    
    return coremap[startframe].addr;
}

// Kernel returns memory
void
ram_returnmem(vaddr_t addr) {
    int i, startframe = -1;
    
    for(i = 0; i < cm_totalframes; ++i) {
        if(PADDR_TO_KVADDR(coremap[i].addr) == addr) {
            startframe = i;
            break;
        }
    }
    
    if (startframe == -1) {
        kprintf("ERROR 0x%x is not allocated\n", addr);
    }
    assert(startframe != -1);
    
    ram_removeframe(startframe);
}

// Borrow some memory as user, signs the pid and address
paddr_t
ram_borrowmemuser(unsigned long npages, int pid, vaddr_t vaddr) {
    paddr_t paddr = ram_borrowmem(npages);
    struct coremap_entry* cmentry = cm_getcmentryfromaddress(paddr);
    
    // No more physical memory left
    if(cmentry == NULL)
        return NULL;
    
    assert(cmentry != NULL);

//    kprintf("PID %d VADDR 0x%x CMENTRY 0x%x\n", pid, vaddr, &cmentry);

    cmentry->usedby = CM_USED;
    cmentry->pid = pid;
    cmentry->vaddr = vaddr;
    cmentry->usecount = 1;
    
    return paddr;
}

// Zeros out a memory at a segment
void
ram_zeromem(int pid, vaddr_t addr) {
    int i, startframe = -1, npages = -1;
    
    for(i = 0; i < cm_totalframes; ++i) {
        if(coremap[i].vaddr == addr) {
            startframe = i;
            npages = coremap[i].length;
            break;
        }
    }
    
    if (startframe == -1) {
        kprintf("ERROR 0x%x is not allocated\n", addr);
    }
    assert(startframe != -1);
    
    ram_zeroframe(startframe);
}

// User returns memory using PID and virtual address
void
ram_returnmemuser(int pid, vaddr_t addr) {
    int i, startframe = -1, npages = -1;
    
    for(i = 0; i < cm_totalframes; ++i) {
        if(coremap[i].vaddr == addr) {
            startframe = i;
            npages = coremap[i].length;
            break;
        }
    }
    
    // Another process is using the memory
    if(coremap[startframe].usecount > 1) {
        coremap[startframe].usecount--;
        return;
    }
    
    if (startframe == -1) {
        kprintf("ERROR 0x%x is not allocated\n", addr);
    }
    assert(startframe != -1);
    
    ram_removeframe(startframe);
}

// User increment memory usecount using frame number
void
ram_incrementframe(int frame) {
    // TODO some error checking
    int i;
    int npages = coremap[frame].length;
    coremap[frame].usecount++;
}

// User returns memory using frame number
void
ram_removeframe(int frame) {
    // TODO some error checking
    int i;
    int npages = coremap[frame].length;
    
    for (i = frame; i < frame + npages; ++i) {
        coremap[i].usedby = CM_FREE;
        coremap[i].vaddr = 0;
        coremap[i].pid = 0;
    }
}

// User zeros frame
void
ram_zeroframe(int frame) {
    // TODO some error checking
    int i;
    int npages = coremap[frame].length;
    
    for (i = frame; i < frame + npages; ++i) {
        bzero(PADDR_TO_KVADDR(coremap[i].addr), PAGE_SIZE);
    }
}

/*
 * This function is intended to be called by the VM system when it
 * initializes in order to find out what memory it has available to
 * manage.
 */
void
ram_getsize(u_int32_t *lo, u_int32_t *hi) {
    *lo = firstpaddr;
    *hi = lastpaddr;
}
