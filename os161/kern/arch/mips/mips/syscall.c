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

#include "addrspace.h"
#include <synch.h>
#include <hashtable.h>

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
            retval = (pid_t) sys_fork(tf);
            err = 0;
            break;
        case SYS_waitpid:
            err = sys_waitpid(tf);
            retval = tf->tf_a1;
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

    // Stack Area
    u_int32_t stacktop = USERSTACK;
    u_int32_t sp = tf->tf_sp;

    // Heap Area
    u_int32_t heapbottom = curthread->t_vmspace->as_vbase1;
    u_int32_t heaptop = curthread->t_vmspace->as_npages1 * PAGE_SIZE + heapbottom;

    int valid = 0;

    // Data in Stack
    if ((u_int32_t) buf > (u_int32_t) sp && (u_int32_t) buf < (u_int32_t) stacktop) {
        valid = 1;
    }
    // Data in Heap
    if ((u_int32_t) buf > (u_int32_t) heapbottom && (u_int32_t) buf < (u_int32_t) heaptop) {
        valid = 1;
    }

    if (!valid) {
        kprintf("Buf 0x%x SP 0x%x\n", (int) buf, (int) sp);
        return EFAULT;
    }

    char buf2[size / sizeof (char) + 1];
    memcpy(buf2, buf, size);
    buf2[size / sizeof (char)] = '\0';
    kprintf("%s", buf2);
    return 0;
}

int
sys_read(struct trapframe *tf) {
    int filehandle = tf->tf_a0;
    char *buf = (char *) tf->tf_a1;

    if (filehandle != STDIN_FILENO) return EBADF;

    // Stack Area
    u_int32_t stacktop = USERSTACK;
    u_int32_t sp = tf->tf_sp;

    // Heap Area
    u_int32_t heapbottom = curthread->t_vmspace->as_vbase1;
    u_int32_t heaptop = curthread->t_vmspace->as_npages1 * PAGE_SIZE + heapbottom;

    int valid = 0;

    // Data in Stack
    if ((u_int32_t) buf > (u_int32_t) sp && (u_int32_t) buf < (u_int32_t) stacktop) {
        valid = 1;
    }
    // Data in Heap
    if ((u_int32_t) buf > (u_int32_t) heapbottom && (u_int32_t) buf < (u_int32_t) heaptop) {
        valid = 1;
    }

    if (!valid) {
        kprintf("Buf 0x%x SP 0x%x\n", (int) buf, (int) sp);
        return EFAULT;
    }


    *buf = getch();
    putch(*buf);
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
    
    // Warp to user mode.
    struct trapframe tfchild2 = *tfchild;
    mips_usermode(&tfchild2);

    /* md_usermode does not return */
    panic("md_usermode returned\n");
}


pid_t sys_fork(struct trapframe *tf) {
    // Make a copy of the address space
    struct addrspace* addrchild = kmalloc(sizeof(struct addrspace));
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
    
    int result = thread_fork(curthread->t_name, argv, 2, child_fork, NULL);
    if (result) {
        kprintf("thread_fork failed: %s\n", strerror(result));
        return result;
    }
    
    return 1; // Parent returns PID of child
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
int sys_waitpid(struct trapframe *tf) {
    pid_t pid = (pid_t) tf->tf_a0;
    int *returncode = (int *) tf->tf_a1;
    int flags = (int) tf->tf_a1;

    if ((unsigned)pid == curthread->pid || (unsigned)pid == curthread->parent_pid) {
        return 0;
    }
        
    
    //P(pids_sem);
    P(wait_pid_sem);
    if (exited_pids[pid].exited == 0) {
        //V(pids_sem);
        //P(wait_pid_sem);
        struct semaphore *sem = sem_create("sem", 0);
        exited_pids[pid].sem = sem;
        V(wait_pid_sem);
        P(sem);
        sem_destroy(sem);
        P(wait_pid_sem);
        exited_pids[pid].sem = NULL;
        V(wait_pid_sem);
    }
    V(wait_pid_sem);
    
    return 0;
}

/*
 * exit() system call.
 *
 */
int
sys_exit(int exit) {
//    kprintf("Thread exited\n");
    
    thread_exit();
    return EINVAL; // Thread exits here
}

int
sys_execv(struct trapframe *tf) {
    
}
