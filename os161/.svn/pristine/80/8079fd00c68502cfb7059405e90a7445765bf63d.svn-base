#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);

/* Required. */
int sys_exit(int code);
int sys_execv(const char *prog, char *const *args);
pid_t sys_fork(void);
int sys_waitpid(pid_t pid, int *returncode, int flags);
/* 
 * Open actually takes either two or three args: the optional third
 * arg is the file mode used for creation. Unless you're implementing
 * security and permissions, you can ignore it.
 */
int sys_open(const char *filename, int flags, ...);
int sys_read(int filehandle, void *buf, size_t size);
int sys_write(int filehandle, const void *buf, size_t size);
int sys_close(int filehandle);
int sys_reboot(int code);
int sys_sync(void);
/* mkdir - see sys/stat.h */
int sys_rmdir(const char *dirname);

/* Recommended. */
int sys_getpid(void);
int sys_ioctl(int filehandle, int code, void *buf);
int sys_lseek(int filehandle, off_t pos, int code);
int sys_fsync(int filehandle);
int sys_ftruncate(int filehandle, off_t size);
int sys_remove(const char *filename);
int sys_rename(const char *oldfile, const char *newfile);
int sys_link(const char *oldfile, const char *newfile);
/* fstat - see sys/stat.h */
int sys_chdir(const char *path);

#endif /* _SYSCALL_H_ */
