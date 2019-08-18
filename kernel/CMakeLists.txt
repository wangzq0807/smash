
enable_language (ASM)

# 编译选项
add_compile_options (-g -O2 -m32 -Wall -Werror -ffreestanding -nostdinc -fno-builtin)
# 
set(KLOG_LEVEL 1)
option(BOCHS_IODEBUG "Enable bochs io debugger" ON)
option(KLOG_ENABLE "Enable kernel log" ON)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
				${CMAKE_CURRENT_SOURCE_DIR}/config.h)

# 链接选项
set (CMAKE_ASM_LINK_FLAGS "-m elf_i386 -Ttext 0x0")

# 使用ld作为链接器
set(CMAKE_C_LINK_EXECUTABLE
    "<CMAKE_LINKER> <CMAKE_ASM_LINK_FLAGS> <OBJECTS> -o <TARGET>")
set(CMAKE_ASM_LINK_EXECUTABLE
    "<CMAKE_LINKER> <CMAKE_ASM_LINK_FLAGS> <OBJECTS> -o <TARGET>")

include_directories (. include)

# 源文件(head.S放最前面)
set (KERNAL_SRCS
	./arch/boot/head.S
	./arch/intr.S
	../usr/core/syscalls.S
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
	../common/lock.c
	../common/random.c
	./lib/log.c
	./dev/char/keyboard.c
	./dev/char/console.c
	./dev/char/tty.c
	./dev/char/serial.c
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
add_executable (smash
    ${KERNAL_SRCS})

