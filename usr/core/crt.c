#include "unistd.h"

extern int main();

int
_start(int argc, char **argv)
{
    int r = main(argc, argv);
    exit(r);
    return 0;
}
