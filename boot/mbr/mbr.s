/* 系统启动时，会将软盘或硬盘第一个启动扇区（512字节）加载到0x7c00位置
 * 然后跳转到0x7c00位置.
 * 1. 从内存将boot代码移动到(BOOTSEG2, 0)
 * 2. 从磁盘将第2扇区后的代码移动到(HEADSEG, 0),保留bios中断[0, HEADSEG*16]
 * 3. 从内存将(HEADSEG, 0)处代码移动到0x00
 * 4. 开启保护模式
 * 5. 跳转到虚拟地址(0x8:0x0)处开始执行
 */
 /* jmp 标号 : 按标号地址与当前地址的差值进行寻址? */
 /* lgdt/movw 标号 : 按标号地址进行寻址? */
BOOTSEG     = 0x07C0
BOOTSEG2    = 0x9000    /* 预留足够多的空间[64K, 576K]来读入loader */
HEADSEG     = 0x1000
HEADLEN     = 254		/* 254 sector = 127KB */
.code16

/*代码段*/
.text
.global _start
_start:
    /* 将boot移动到(BOOTSEG2, 0)处 */
    movw $BOOTSEG, %ax
    movw %ax, %ds
    movw $BOOTSEG2, %ax
    movw %ax, %es
    cli
    cld
    xorw %si, %si
    xorw %di, %di
    movw $256, %cx
    rep movsw
    jmpl $BOOTSEG2, $(_start2-_start)
_start2:
    movw %cs, %ax
    movw %ax, %ds
    movw %ax, %es
    sti						/* 允许中断 */
    /* 文字模式, 80*25, 16色 */
    movw $0x03, %ax
    int $0x10
test_lba:
    /*********************
     * INT13h Extensions
     * 测试是否支持 LBA
     *********************/
    movw $0x4100, %ax	/* AH = 0x41 */
    movw $0x55AA, %bx	/* BX = 0x55AA */
    movw $0x80, %cx		/* CL = 0x80 */
    int  $0x13
    jc lba_error
    jmp lba_read

lba_error:
    xorw %ax, %ax
    jmp lba_error

lba_pack:
    .word 0x0010	/* lba_pack的长度16 byte */
    .word HEADLEN	/* read 1024 sector */
    .word 0x0000	/* mem buffer 段内偏移 */
    .word HEADSEG   /* mem buffer (HEADSEG, 0) */
    .long 0x1		/* LBA的32位低地址 */
    .long 0x0

lba_read:
    /*********************
     * INT13h Extensions
     * LBA读磁盘
     *********************/
     movw $0x4200, %ax	/* AH = 0x42 */
     movw $(lba_pack - _start), %bx	/* DS:SI 指向 lba_pack */
     movw %bx, %si
     movw $0x80, %cx	/* CL = 0x80 */
     int  $0x13
     jnc read_ok

read_failed:
    xorw %bx, %bx
    jmp read_failed

read_ok:
    /***********************
     * 将0x10000处代码移动到0x00
     ***********************/
    cli						/* 关闭中断 */
    cld						/* 清rep指令方向，使源为ds:si,目的: es:di */
    movw $HEADSEG, %ax
    xorw %bx, %bx
_mv_one_sector:
    cmp $(HEADLEN<<5), %bx
    je protect_mode
    movw %ax, %ds			/* ds = 0x1000 */
    movw %bx, %es			/* es = 0x0000 */
    xorw %si, %si
    xorw %di, %di
    movw $256, %cx			/* 移动 256 次 */
    rep movsw 				/* 每次2字节 */
    add $0x20, %ax
    add $0x20, %bx
    jmp _mv_one_sector

protect_mode:
    /* 设置gdtr和idtr */
    movw $BOOTSEG2, %ax
    movw %ax, %ds
    lidt idt_ptr-_start
    lgdt gdt_ptr-_start
    /* 开启保护模式 */
    movl %cr0, %eax
    orl  $1, %eax
    movl %eax, %cr0
    /* 设置cs和ip */
    jmpl $8, $0				/* 8为gdt代码段，基地址为0x0000,偏移地址为0x0000 */

gdt_table: .word 0, 0, 0, 0
    /* 代码段 */
    .word 0xFFFF			/* 段限长 4GB */
    .word 0x0000			/* 基地址 0x0000 */
    .word 0x9a00			/* 1001 : 段存在,特权00，系统段；1011:代码段，非一致，可读可执行，已访问  */
    .word 0x00CF			/* 粒度4K */
    /* 数据段 */
    .word 0xFFFF			/* 段限长 4GB */
    .word 0x0000			/* 基地址 0x0000 */
    .word 0x9200			/* 数据段 */
    .word 0x00CF			/* 粒度4K */

idt_ptr: .word 0
    .word 0, 0

gdt_ptr: .word 0x7ff			/* 2^11 = 2048, 256项 */
    .long (gdt_table-_start) + (BOOTSEG2<<4)	/* base = 0x7xxx */

