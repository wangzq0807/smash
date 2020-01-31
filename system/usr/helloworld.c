#include "stdio.h"

int
main(int argc, const char **argv)
{
    printf("Hello,World!\n");
    while (argc--) {
        printf("%s\n", argv[argc]);
    }
    return 0;
}