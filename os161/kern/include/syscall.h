#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);
int sys_exit(int code);
int sys_execv(struct trapframe *tf);
pid_t sys_fork(struct trapframe *tf);
int sys_waitpid(struct trapframe *tf);
int sys_open(const char *filename, int flags, ...);
int sys_read(struct trapframe *tf);
int sys_write(struct trapframe *tf);
int sys_close(int filehandle);
int sys_reboot(int code);
int sys_sync(void);
int sys_rmdir(const char *dirname);
int sys_getpid(struct trapframe *tf);
int sys_ioctl(int filehandle, int code, void *buf);
int sys_lseek(int filehandle, off_t pos, int code);
int sys_fsync(int filehandle);
int sys_ftruncate(int filehandle, off_t size);
int sys_remove(const char *filename);
int sys_rename(const char *oldfile, const char *newfile);
int sys_link(const char *oldfile, const char *newfile);
int sys_chdir(const char *path);

void syscall_bootstrap(void);

extern struct lock *execv_lock;

#endif /* _SYSCALL_H_ */
