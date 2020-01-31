#include "unistd.h"
#include "string.h"
#include "stdio.h"

int
main(int argc, const char *argv[])
{
    char buf[1025] = {0};
    read(stdin, buf, 1024);
    buf[1024] = 0;
    printf("pipecho: %s", buf);

    return 0;
}