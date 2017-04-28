#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <lib.h>

void hello() {
    kprintf("hello world\n");
}