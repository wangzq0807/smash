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

    call start_main

loop1:
    jmp loop1

/* 4KB 堆栈 */
.align  4
    .fill 1024, 4, 0
init_stack:
    .long init_stack
    .word DATA_SEG

