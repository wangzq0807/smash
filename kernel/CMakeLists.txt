
enable_language (ASM)

# 编译选项
# -nostdinc: 不使用系统头文件
# -nostdlib, -nostartfiles: 不使用系统库,标准启动文件(_start)
set(CMAKE_C_FLAGS "-g -O2 -m32 -Wall -Werror -ffreestanding -nostdinc -nostdlib")
set(CMAKE_C_LINK_FLAGS "-m32 -T ${CMAKE_CURRENT_SOURCE_DIR}/smash.ld")
set(CMAKE_ASM_FLAGS "-m32")

# 
set(KLOG_LEVEL 0)
option(BOCHS_IODEBUG "Enable bochs io debugger" ON)
option(KLOG_ENABLE "Enable kernel log" ON)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
				${CMAKE_CURRENT_SOURCE_DIR}/config.h)

include_directories (. include ../include)

set (KERNAL_SRCS
	./arch/boot.S
	./main.c
	./arch/intr.S
	../usr/core/syscalls.S
	./knl/sys_file.c
	./knl/sys_fork.c
	./knl/syscalls.c
	./knl/sys_exec.c
	./knl/sys_mem.c

	./arch/x86.c
	./arch/page.c
	./arch/task.c
	./arch/utc.c
	./arch/irq.c
	./arch/arch.c
	./lib/lock.c
	./lib/random.c
	./lib/log.c
	./lib/bitmap.c
	./lib/list.c
	./lib/hashmap.c
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
	./mem/memory.c
	./mem/pymem.c
	./lib/elf.c)
# 目标kernel
add_executable (smash
    ${KERNAL_SRCS})

