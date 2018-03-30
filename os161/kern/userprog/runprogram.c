/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than this function does.
 */

#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <thread.h>
#include <curthread.h>
#include <vm.h>
#include <vfs.h>
#include <test.h>
#include <coremap.h>

/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */
int
runprogram(char *progname, int argc, char** argv) {
    DEBUG(DB_VM, "runprogram\n");
    
    struct vnode *v;
    vaddr_t entrypoint, stackptr;
    int result;

    /* Open the file. */
    result = vfs_open(progname, O_RDONLY, &v);
    if (result) {
        return result;
    }

    /* We should be a new thread. */
    assert(curthread->t_vmspace == NULL);
    
    /* Create a new address space. */
    curthread->t_vmspace = as_create();
    if (curthread->t_vmspace == NULL) {
        vfs_close(v);
        return ENOMEM;
    }

    /* Activate it. */
    as_activate(curthread->t_vmspace);

    /* Load the executable. */
    result = load_elf(v, &entrypoint);
    if (result) {
        /* thread_exit destroys curthread->t_vmspace */
        vfs_close(v);
        return result;
    }

    /* Done with the file now. */
    vfs_close(v);

    /* Define the user stack in the address space */
    result = as_define_stack(curthread->t_vmspace, &stackptr);
    if (result) {
        /* thread_exit destroys curthread->t_vmspace */
        return result;
    }

    // array to hold user space addresses of the args
    char* user_space_addr[argc+1];

    // copy args into user space stack
    int i;
    for (i = argc - 1; i >= 0; i--) {
        char* s = kstrdup(argv[i]);
        s[strlen(s)] = '\0';
        stackptr = stackptr - (strlen(s) + 1);
        user_space_addr[i] = (char*) stackptr;
        copyout(s, (userptr_t) stackptr, strlen(s) + 1);
        kfree(s);
    }
    
    // set last element to NULL
    user_space_addr[argc] = NULL;

    // align stack
    stackptr = stackptr - (stackptr % 4);

    // copy array of pointers to args to the user space
    stackptr = stackptr - ((argc+1) * sizeof (char*));
    copyout(user_space_addr, (userptr_t) stackptr, sizeof (user_space_addr));

    md_usermode(argc, (userptr_t) stackptr, stackptr, entrypoint);

    /* md_usermode does not return */
    panic("md_usermode returned\n");
    return EINVAL;
}

int
closeprogram(char *progname) {
    /* Ends and goes back to kernel mode*/
    (void) progname;
    
    return EINVAL;
}
