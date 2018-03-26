#ifndef COREMAP_H
#define COREMAP_H

#define CM_FREE     0
#define CM_USED     1
#define CM_KERNEL   2
#define CM_COREMAP  3

struct coremap_entry {
    paddr_t addr;       // Physical Address
    unsigned usedby;
    unsigned pid;       // PID of process using this physical address
    vaddr_t vaddr;      // Virtual Address
};

extern struct coremap_entry *coremap;
extern int cm_totalframes;

void coremap_bootstrap();
void coremap_getkernelusage();
void cm_print();

#endif /* COREMAP_H */

