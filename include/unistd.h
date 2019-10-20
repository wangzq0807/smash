#ifndef __UNISTD_H__
#define __UNISTD_H__

#include "sys/types.h"
#define O_RDONLY    0x1
#define O_WRONLY    0x2
#define O_NONBLOCK  0x4
#define O_APPEND    0x8

#define O_CREAT     0x200
#define O_TRUNC     0x400

#define O_RDWR      (O_RDONLY|O_WRONLY)

extern int exit(int code);
extern int fork(void);
extern int read(int fd, void *buf, int count);
extern int write(int fd, const void *buf, int count);
extern int open(const char *pathname, int flags, int mode);
extern int close(int fd);
extern int waitpid(int pid, int *status, int options);
extern int creat(const char *pathname, int mode);
extern int link(const char *oldpath, const char *newpath);
extern int unlink(const char *pathname);
extern int execve(const char *pathname, char *const argv[], char *const envp[]);
extern int chdir(const char *pathname);
extern int time(int *tloc);
extern int mknod(const char *pathname, int mode, int dev);
extern int chmod(const char *pathname, int mode);
extern int chown(const char *pathname, int owner, int group);

extern int mkdir(const char *pathname, int mode);
extern int rmdir(const char *pathname);
extern int pause(void);
extern int getpid(void);
extern int pipe(int fd[2]);
extern int dup(int fd);

void *mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset);

#endif // __UNISTD_H__
