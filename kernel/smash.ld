OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    . = 0xFE000000;
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }
    __init_array_start = .;
    __init_array_end = .;
}
