#include "stdio.h"
#include "unistd.h"
#include "sys/fcntl.h"

int
main(int argc, const char *argv[])
{
    if (argc < 1) {
        printf("USAGE: cat filepath\n");
        return 0;
    }
    int fd = open(argv[0], O_RDONLY, 0);
    if (fd == -1) {
        printf("can not open file\n");
    }
    char buf[1025] = {0};
    while (1) {
        int rdcnt = read(fd, buf, 1024);
        buf[rdcnt] = 0;
        printf(buf);
        if (rdcnt < 1024)
            break;
    }
    close(fd);
    return 0;
}