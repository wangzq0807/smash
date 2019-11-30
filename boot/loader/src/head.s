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

/* 4KB 栈(方便检测栈溢出) */
.org 4096
.global _PDT_
_PDT_:  /* 页目录表 */
init_stack:
    .long init_stack
    .word DATA_SEG

.org 8192
/* 第一个页表 */
.global _PT0_
_PT0_:
    .fill 1024, 4, 0

