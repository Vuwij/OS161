#ifndef COREMAP_H
#define COREMAP_H

#define CM_FREE     0
#define CM_USED     1
#define CM_KERNEL   2
#define CM_KTEMP    3   // Temporary kernel memory for kallocs etc
#define CM_COREMAP  4

struct coremap_entry {
    paddr_t addr;       // Physical Address
    unsigned usedby;    // What is the memory segment used by
    unsigned pid;       // PID of process using this physical address
    unsigned usecount;  // Number of processes using the entry
    vaddr_t vaddr;      // Virtual Address
    unsigned length;    // Length of coremap (for page allocations greater than single page)
};

extern struct coremap_entry *coremap;
extern paddr_t firstpaddr;
extern unsigned cm_totalframes;
extern unsigned cm_totalkernelframes;


void coremap_bootstrap();
void coremap_getkernelusage();
unsigned cm_getframefromaddress(paddr_t paddr);
struct coremap_entry* cm_getcmentryfromaddress(paddr_t paddr);
void cm_print();

#endif /* COREMAP_H */

