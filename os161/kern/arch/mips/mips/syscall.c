#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <machine/pcb.h>
#include <machine/spl.h>
#include <machine/trapframe.h>
#include <kern/callno.h>
#include <kern/unistd.h>
#include <syscall.h>
#include <thread.h>
#include <curthread.h>
#include <vm.h>
#include <kern/limits.h>
#include <kern/../types.h>
#include <vfs.h>
#include "addrspace.h"
#include <synch.h>
#include <hashtable.h>
#include <clock.h>

#define MENU_CALL 1

/*
 * System call handler.
 *
 * A pointer to the trapframe created during exception entry (in
 * exception.S) is passed in.
 *
 * The calling conventions for syscalls are as follows: Like ordinary
 * function calls, the first 4 32-bit arguments are passed in the 4
 * argument registers a0-a3. In addition, the system call number is
 * passed in the v0 register.
 *
 * On successful return, the return value is passed back in the v0
 * register, like an ordinary function call, and the a3 register is
 * also set to 0 to indicate success.
 *
 * On an error return, the error code is passed back in the v0
 * register, and the a3 register is set to 1 to indicate failure.
 * (Userlevel code takes care of storing the error code in errno and
 * returning the value -1 from the actual userlevel syscall function.
 * See src/lib/libc/syscalls.S and related files.)
 *
 * Upon syscall return the program counter stored in the trapframe
 * must be incremented by one instruction; otherwise the exception
 * return code will restart the "syscall" instruction and the system
 * call will repeat forever.
 *
 * Since none of the OS/161 system calls have more than 4 arguments,
 * there should be no need to fetch additional arguments from the
 * user-level stack.
 *
 * Watch out: if you make system calls that have 64-bit quantities as
 * arguments, they will get passed in pairs of registers, and not
 * necessarily in the way you expect. We recommend you don't do it.
 * (In fact, we recommend you don't use 64-bit quantities at all. See
 * arch/mips/include/types.h.)
 */

void
mips_syscall(struct trapframe *tf) {
    int callno;
    int32_t retval;
    int err;

    assert(curspl == 0);

    callno = tf->tf_v0;

    /*
     * Initialize retval to 0. Many of the system calls don't
     * really return a value, just 0 for success and -1 on
     * error. Since retval is the value returned on success,
     * initialize it to 0 by default; thus it's not necessary to
     * deal with it except for calls that return other values, 
     * like write.
     */

    retval = 0;

    switch (callno) {
        case SYS__exit:
            err = sys_exit(tf->tf_a0);
            break;
        case SYS_reboot:
            err = sys_reboot(tf->tf_a0);
            break;
        case SYS_fork:
            err = sys_fork(tf);
            retval = tf->tf_a0;
            break;
        case SYS_waitpid:
            err = sys_waitpid(tf,0);
            retval = tf->tf_a0;    // return pid
            break;
        case SYS_open:
            break;
        case SYS_read:
            err = sys_read(tf);
            retval = tf->tf_a2;
            break;
        case SYS_write:
            err = sys_write(tf);
            retval = tf->tf_a2;
            break;
        case SYS_close:
            break;
        case SYS_sync:
            break;
        case SYS_sbrk:
            err = sys_sbrk(tf->tf_a0, &retval);
            break;
        case SYS_getpid:
            retval = sys_getpid(tf);
            err = 0;
            break;
        case SYS_ioctl:
            break;
        case SYS_lseek:
            break;
        case SYS_fsync:
            break;
        case SYS_ftruncate:
            break;
        case SYS_fstat:
            break;
        case SYS_execv:
            err = sys_execv(tf);
            break;
        case SYS___time:
            err = sys___time(tf, &retval); 
            break;
        default:
            kprintf("Unknown syscall %d\n", callno);
            err = ENOSYS;
            break;
    }


    if (err) {
        /*
         * Return the error code. This gets converted at
         * userlevel to a return value of -1 and the error
         * code in errno.
         */
        tf->tf_v0 = err;
        tf->tf_a3 = 1; /* signal an error */
    } else {
        /* Success. */
        tf->tf_v0 = retval;
        tf->tf_a3 = 0; /* signal no error */
    }

    /*
     * Now, advance the program counter, to avoid restarting
     * the syscall over and over again.
     */

    tf->tf_epc += 4;

    /* Make sure the syscall code didn't forget to lower spl */
    assert(curspl == 0);
}

void syscall_bootstrap(void) {
}

/*
 * write() system call.
 *
 */
int
sys_write(struct trapframe *tf) {
    int filehandle = tf->tf_a0;
    char *buf = (char *) tf->tf_a1;
    int size = tf->tf_a2;

    // Validate errors
    if (filehandle != STDOUT_FILENO && filehandle != STDERR_FILENO) return EBADF;
 
    char buf2[size + 1];
    size_t actual;
    int err = copyinstr((const_userptr_t) buf, buf2, size + 5, &actual);
    if(err) return err;
    buf2[size] = '\0';
    kprintf("%s", buf2);
    return 0;
}

int
sys_read(struct trapframe *tf) {
    int filehandle = tf->tf_a0;
    char *buf = (char *) tf->tf_a1;
    size_t size = (size_t) tf->tf_a2;

    if (filehandle != STDIN_FILENO) return EBADF;
    
    char kbuf = getch();
    size_t actual;
    int err = copyoutstr( &kbuf, (userptr_t) buf, size + 1, &actual);
    if(err) return err;
    
    putch(kbuf);
    return 0;
}

static
void
child_fork(void *ptr, unsigned long nargs) {
    (void) nargs;
    
    unsigned* argv = (unsigned*) ptr;
    struct addrspace* addrspace2 = (struct addrspace*) argv[0];
    struct trapframe* tfchild = (struct trapframe*) argv[1];
    
    // Create a new address space and activate
    curthread->t_vmspace = addrspace2;
    if (curthread->t_vmspace == NULL) {
        panic("Child fork failed\n");
    }
    as_activate(curthread->t_vmspace);
    assert(curspl == 0);
    
    // Create a new trapframe
    tfchild->tf_a3 = 0; // Return Value
    tfchild->tf_v0 = 0; // Error Signal
    tfchild->tf_epc += 4; // Advance Program Counter
    
    // Free kernel memory
    struct trapframe tfchild2 = *tfchild;
    kfree(tfchild);
    kfree(ptr);
    
    // Warp to user mode.
    mips_usermode(&tfchild2);

    /* md_usermode does not return */
    panic("md_usermode returned\n");
}


int sys_fork(struct trapframe *tf) {
    // Make a copy of the address space
    struct addrspace* addrchild;
    int err = as_copy(curthread->t_vmspace, &addrchild);
    if(err) return ENOMEM;
    
    // Make a copy of the trapframe
    struct trapframe* tfchild = kmalloc(sizeof(struct trapframe));
    memcpy(tfchild, tf, sizeof(struct trapframe));
    if(tfchild == NULL) return ENOMEM;
    
    // Pass the arguments into argv
    unsigned *argv = kmalloc(sizeof(unsigned) * 2);
    if(argv == NULL) return ENOMEM;
    argv[0] = (unsigned) addrchild;
    argv[1] = (unsigned) tfchild;
    
    struct thread * childthread;
    int result = thread_fork(curthread->t_name, argv, 2, child_fork, &childthread);
    if (result) {
        kprintf("thread_fork failed: %s\n", strerror(result));
        return result;
    }
    
    tf->tf_a0 = childthread->pid; // Parent returns PID of child
    return 0;
}

/*
 * getpid() system call.
 *
 */
int sys_getpid(struct trapframe *tf) {
    (void) tf;
    return curthread->pid;
}

/*
 * waitpid() system call.
 *
 */
int sys_waitpid(struct trapframe *tf, int call) {
    pid_t pid = (pid_t) tf->tf_a0;
    int *returncode = (int *) tf->tf_a1;
    int flags = (int) tf->tf_a2;
    
    if(pid == 0)
        return EINVAL;
    
    if (call != MENU_CALL) {
        char test[4];
        if(copyin((const_userptr_t) returncode, &test, 1)) {
            return EFAULT;
        }
    }
    
    if(flags != 0)
        return EINVAL;
    
    // Cannot wait for current and parent PID
    if ((unsigned)pid == curthread->pid || (unsigned)pid == curthread->parent_pid) {
       return EINVAL;
    }
    
    // Can only wait for its old child
    if (!exists(curthread->child_pid, pid))
        return EINVAL;
    
    lock_acquire(pidtablelock);
    
    // Check if child PID is in PID table, if not, return
    if(!ht_get_val(&pidlist, pid)) {
        lock_release(pidtablelock);
        return EINVAL;
    }
    
    // Check if child PID has exited
    if(exitcodes[pid] != -1000) {
        int childexitcode = exitcodes[pid];
        exitcodes[pid] = -1000;
        ht_remove(&pidlist, pid);
        remove_val(&curthread->child_pid, pid);
        lock_release(pidtablelock);
        
        copyout( &childexitcode, (userptr_t) returncode, sizeof(int));
        return 0;
    }
    
    // Create the PID CV if it doesn't exist
    if(waitpid[pid] == NULL) {
        waitpid[pid] = cv_create("cv");
    }
    
    // While the PID is still in the PID table
    while(exitcodes[pid] == -1000) {
        cv_wait(waitpid[pid], pidtablelock);
    }  
    
    lock_acquire(pidtablelock);
    int childexitcode = exitcodes[pid];
    exitcodes[pid] = -1000;
    ht_remove(&pidlist, pid);
    remove_val(&curthread->child_pid, pid);
    lock_release(pidtablelock);
    
    copyout( &childexitcode, (userptr_t) returncode, sizeof(int));
    
    kprintf("Finished waiting for PID %d\n", pid);
    return 0;
}

/*
 * exit() system call.
 *
 */
int
sys_exit(int exitcode) {
    kprintf("PID %d exited\n", curthread->pid);
    // Create an exit code
    lock_acquire(pidtablelock);
    exitcodes[curthread->pid] = exitcode;
    
    // Signal any sleeping threads
    if(waitpid[curthread->pid] != NULL) {
        lock_release(pidtablelock);
        cv_broadcast(waitpid[curthread->pid], pidtablelock);
    }
    lock_release(pidtablelock);
    
    thread_exit();
    
    return EINVAL;
}

#define MAX_ARG 15

/*
 * sys_execv() system call.
 *
 */
int
sys_execv(struct trapframe *tf) {
    char *progname = (char *) tf->tf_a0;
    char **argv = (char **) tf->tf_a1;    
    char *prognamek = kmalloc(sizeof(char) * PATH_MAX);
    char **argvk = (char **) kmalloc(sizeof(char*) * MAX_ARG);
    int i;
    for (i = 0; i < MAX_ARG; ++i) {
        argvk[i] = (char *) kmalloc(sizeof(char) * PATH_MAX);
    }
    
    // Copy in the program name and arguments
    size_t actual; 
    if(copyinstr((const_userptr_t) progname, prognamek, PATH_MAX, &actual)) {
        return EFAULT; // Bad program name
    }
    
    if(strcmp(prognamek, "") == 0)
        return EINVAL;
    
    if(argv == NULL)
        return EFAULT;
    
    char test[4];
    if(copyin((const_userptr_t) argv, &test, 1)) {
        return EFAULT;
    }
    
    i = 0;
    while(1) {
        if(argv[i] == NULL) break;
        int err = copyinstr((const_userptr_t) argv[i], argvk[i], PATH_MAX, &actual);
        if(err == EFAULT) {
            return EFAULT;
        }
        ++i;
    }
    int argc = i;
    
    
    // Open the program
    struct vnode *v;
    int err = vfs_open(prognamek, O_RDONLY, &v);
    if(err) {
        kfree(prognamek);
        for (i = 0; i < MAX_ARG; ++i) {
            kfree(argvk[i]);
        }
        kfree(argvk);
        return err;
    }
    
    // Reset the address space
    as_reset(curthread->t_vmspace);
    as_activate(curthread->t_vmspace);
    curthread->t_vmspace->progfile = v;
    
    // Load file into address space
    vaddr_t entrypoint, stackptr;
    
    strcpy(curthread->t_vmspace->progname, prognamek);
    curthread->t_vmspace->progfile = v;
    err = load_elf(v, &entrypoint);
    if(err) {
        kfree(prognamek);
        for (i = 0; i < MAX_ARG; ++i) {
            kfree(argvk[i]);
        }
        kfree(argvk);
        return err;
    }
        
    /* Define the user stack in the address space */
    err = as_define_stack(curthread->t_vmspace, &stackptr);
    if (err) {
        kfree(prognamek);
        for (i = 0; i < MAX_ARG; ++i) {
            kfree(argvk[i]);
        }
        kfree(argvk);
        return err;
    }

    // array to hold user space addresses of the args
    char* user_space_addr[argc+1];

    // copy args into user space stack
    for (i = argc - 1; i >= 0; i--) {
        char* s = kstrdup(argvk[i]);
        s[strlen(s)] = '\0';
        stackptr = stackptr - (strlen(s) + 1);
        user_space_addr[i] = (char*) stackptr;
        copyout(s, (userptr_t) stackptr, strlen(s) + 1);
    }
    
    // set last element to NULL
    user_space_addr[argc] = NULL;

    // align stack
    stackptr = stackptr - (stackptr % 4);

    // copy array of pointers to args to the user space
    stackptr = stackptr - ((argc+1) * sizeof (char*));
    copyout(user_space_addr, (userptr_t) stackptr, sizeof (user_space_addr));
    
    // Free kernel memory
    kfree(prognamek);
    for (i = 0; i < MAX_ARG; ++i) {
        kfree(argvk[i]);
    }
    kfree(argvk);
    
    md_usermode(argc, (userptr_t) stackptr, stackptr, entrypoint);
    
    return 0;
}

/*
 * sys___time() system call.
 *
 */
int
sys___time(struct trapframe *tf, int32_t* retval) {
    time_t *seconds = (time_t*) tf->tf_a0;
    u_int32_t *nanoseconds = (u_int32_t *) tf->tf_a1;
    
    time_t kseconds;
    u_int32_t knanoseconds;
    
    gettime(&kseconds, &knanoseconds);
    int err;
    if(seconds != NULL) {
        err = copyout(&kseconds, (userptr_t) seconds, sizeof(time_t));
        if (err) return err;
    }
    if(nanoseconds != NULL) {
        err = copyout(&knanoseconds, (userptr_t) nanoseconds, sizeof(u_int32_t));
        if (err) return err;
    }
    
    *retval = kseconds;
    
    return 0;
}

/*
 * sys___time() system call.
 *
 */
int
sys_sbrk(int increment, int32_t* retval) {
    kprintf("Test\n");
    
    struct addrspace *as  = curthread->t_vmspace;
//    increment += increment
    if (as->as_heap_end + increment >= as->as_heap_start) {
        *retval = as->as_heap_end;
        as->as_heap_end += increment;        
        return 0;
    }
    return EINVAL;        
}
