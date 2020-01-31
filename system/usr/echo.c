#include "stdio.h"

int
main(int argc, const char **argv)
{
    if (argc == 1) {
        printf("%s\n", argv[0]);
    }
    else {
        printf("wrong params!\n");
    }
    return 0;
}