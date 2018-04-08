#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>

static char *hargv[2] = {(char *) "hog", NULL};

int
main(void) {
    
    int pid = fork();
    int status;
    if(pid == 0) {
        waitpid(pid, status, 0);
    }
    else {
        printf("Hog\n");
        execv("/testbin/hog", hargv);
    }

    return 0;
}
