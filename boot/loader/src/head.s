.code32

CODE_SEG = 0x8
DATA_SEG = 0x10

.text
.global _start
_start:
    movl $DATA_SEG, %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    lss init_stack, %esp

    call start_bootloader

/* 4KB 栈(方便检测栈溢出) */
.org 4096
init_stack:
    .long init_stack
    .word DATA_SEG

