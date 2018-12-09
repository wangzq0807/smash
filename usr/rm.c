#include "stdio.h"
#include "unistd.h"
#include "sys/fcntl.h"
#include "sys/types.h"

int
main(int argc, const char *argv[])
{
    if (argc < 1) {
        printf("USAGE: rm filepath\n");
        return 0;
    }
    if (unlink(argv[0]) == -1) {
        printf("can not delete target file\n");
    }
    return 0;
}
