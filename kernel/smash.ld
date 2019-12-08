OUTPUT_FORMAT(elf32-i386)
ENTRY(start_main)

SECTIONS
{
    . = 0x100000;
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }
    __init_array_start = .;
    __init_array_end = .;
}
