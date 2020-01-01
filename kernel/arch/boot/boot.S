#ifndef __MULTIBOOT_S__
#define __MULTIBOOT_S__
#define ASM_FILE
#include "multiboot.h"

#define MULTIBOOT_FLAGS     MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
#define MULTIBOOT_CHKSUM -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_FLAGS)

.text
.global _start
_start:
    jmp _multiboot
    /* multiboot_header */
    .align 4
    .long MULTIBOOT_HEADER_MAGIC
    .long MULTIBOOT_FLAGS
    .long MULTIBOOT_CHKSUM

    .long 0
    .long 0
    .long 0
    .long 0
    .long 0

    .long 0
    .long 0
    .long 0

_multiboot:
    pushl $0
    popf

    pushl %eax
    pushl %ebx
    call boot_trap
_errorloop:
    hlt
    jmp _errorloop

#endif /* __MULTIBOOT_S__ */

