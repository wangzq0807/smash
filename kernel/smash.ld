OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

SECTIONS
{
    _LMA = 0x100000; /* 1M */
    . = _LMA;
    kernel_start = .;
    /* boot负责将内核代码映射到高地址 */
    .boot.text : ALIGN(0x1000) {
        *boot*.o(.text*)
        *boot*.o(.rodata*)
    }
    .boot.data : ALIGN(0x1000) {
        *boot*.o(.data)
    }
    .boot.bss : ALIGN(0x1000) {
        *boot*.o(.bss)
    }
    boot_end = .;

    _VMA = 0xF0000000; /* last 256M */
    . += _VMA;
    /* 内核代码 */
    .text : AT(ADDR(.text) - _VMA) ALIGN(0x1000) {
        *(.text*)
        *(.rodata*)
    }
    
    .data : AT(ADDR(.data) - _VMA) ALIGN(0x1000) {
        *(.data)
    }
    
    .bss : AT(ADDR(.bss) - _VMA) ALIGN(0x1000) {
        *(.bss)
    }

    kernel_end = . - _VMA;
}
