#include "multiboot.h"
#include "sys/types.h"
#include "asm.h"

extern void start_main();
extern char _VMA;
extern char kernel_start;
extern char kernel_end;

void map_kernel_to_heigh_addr();

void boot_trap(uint32_t magic, multiboot_info_t* binfo)
{
    if (magic != MULTIBOOT_HEADER_MAGIC)
        return;
    map_kernel_to_heigh_addr();
    start_main();
}

void map_kernel_to_heigh_addr()
{
     get_cr3();
}
