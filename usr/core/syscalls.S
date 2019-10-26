

#define SYSCALL(num, func)  \
.global func;               \
func:                       \
    movl $num, %eax;        \
    int $0x80;              \
    ret

.text
SYSCALL(1, exit)
SYSCALL(2, fork)
SYSCALL(3, read)
SYSCALL(4, write)
SYSCALL(5, open)
SYSCALL(6, close)
SYSCALL(7, waitpid)
SYSCALL(8, creat)
SYSCALL(9, link)
SYSCALL(10, unlink)
SYSCALL(11, execve)
SYSCALL(12, chdir)
SYSCALL(13, time)
SYSCALL(14, mknod)
SYSCALL(15, chmod)
SYSCALL(16, chown)
SYSCALL(17, mkdir)
SYSCALL(18, rmdir)
SYSCALL(19, pause)
SYSCALL(20, getpid)
SYSCALL(21, pipe)
SYSCALL(22, dup)
SYSCALL(23, mmap)
