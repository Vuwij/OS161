/*
 * hog.c
 * 
 * Spawned by several other user programs to test time-slicing.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(void) {

    volatile int i;
    
    for (i = 0; i < 50000; i++)
        ;
    return 0;
}
