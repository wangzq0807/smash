#include "stdio.h"
#include "unistd.h"
#include "sys/fcntl.h"
#include "sys/types.h"

int
main(int argc, const char *argv[])
{
    if (argc < 2) {
        printf("USAGE: ln target linkname\n");
        return 0;
    }
    if (link(argv[0], argv[1]) == -1) {
        printf("can not create hard link!\n");
    }
    return 0;
}
