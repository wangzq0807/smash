#include "stdio.h"
#include "unistd.h"
#include "sys/fcntl.h"
#include "sys/types.h"

#define FILENAME_LEN 29
struct Direction {
    ino_t       dr_inode;
    char        dr_name[FILENAME_LEN+1];
};

int
main(int argc, const char **argv)
{
    int fd = -1;
    char buf[1024] = {0};
    if (argc > 0) {
        fd = open(argv[argc-1], O_RDONLY, 0);
    }
    else {
        fd = open(".", O_RDONLY, 0);
    }
    if (fd == -1) {
        printf("failed to open target dir\n");
        return 0;
    }

    while (1) {
        int rdcnt = read(fd, buf, 1024);
        struct Direction *dirs = (struct Direction*)buf;
        int i = 0;
        for (; i < rdcnt/sizeof(struct Direction); ++i) {
            if (dirs[i].dr_inode != 0)
                printf("%s\n", dirs[i].dr_name);
        }
        if (rdcnt < 1024)
            break;
    }
    close(fd);
    return 0;
}
