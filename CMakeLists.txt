cmake_minimum_required (VERSION 3.0)
enable_language (ASM)

# 编译选项
add_compile_options (-g -O2 -m32 -Wall -Werror -ffreestanding -nostdinc -fno-builtin)

# 链接选项
set (LD_FLAGS "-m elf_i386 -Ttext 0x0")

# 使用ld作为链接器
set(CMAKE_C_LINK_EXECUTABLE
    "ld ${LD_FLAGS} <OBJECTS> -o <TARGET>")
set(CMAKE_ASM_LINK_EXECUTABLE
    "ld ${LD_FLAGS} <OBJECTS> -o <TARGET>")

include_directories (. include)
add_subdirectory (usr)

# 源文件(head.S放最前面)
set (KERNAL_SRCS
    ./arch/boot/head.S
	./arch/intr.S
	./usr/core/syscalls.S
	./knl/sys_file.c
	./knl/sys_fork.c
	./knl/syscalls.c
	./knl/sys_exec.c
	./main.c
	./arch/x86.c
	./arch/page.c
	./arch/task.c
	./arch/utc.c
	./arch/irq.c
	./arch/arch.c
	./lib/lock.c
	./lib/log.c
	./dev/char/keyboard.c
	./dev/char/console.c
	./dev/char/tty.c
	./fs/superblk.c
	./fs/partion.c
	./fs/inodes.c
	./fs/buffer.c
	./fs/pipe.c
	./fs/file.c
	./fs/vfile.c
	./fs/hard_disk.c
	./fs/path.c
	./fs/zones.c
	./mem/memory.c)
# 目标kernel
add_executable (kernel
    ${KERNAL_SRCS})
add_executable (boot
	./arch/boot/boot.S)

# 创建镜像
# boot只需要代码区
# NOTE:kernel除了代码区以外还需要数据区和bss区等
# NOTE:写入的数据不能超过xxKB
add_custom_target (hd.img ALL)
add_custom_command (TARGET hd.img PRE_BUILD
    COMMAND objcopy -j .text -O binary boot boot.img
    COMMAND objcopy -O binary kernel kernel.img
    COMMAND objdump -S kernel > kernel.sym
	COMMAND dd if=boot.img of=hd.img bs=432 count=1 conv=notrunc
    COMMAND dd if=/dev/zero of=hd.img bs=512 count=120 seek=1 conv=notrunc
    COMMAND dd if=kernel.img of=hd.img bs=512 count=120 seek=1 conv=notrunc)
add_dependencies (hd.img boot kernel)

