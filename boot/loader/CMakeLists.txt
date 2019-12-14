enable_language (ASM)

include(CMakeASMInformation)

# 链接选项
set (CMAKE_ASM_LINK_FLAGS "-m elf_i386 -Ttext 0x1000")

# 使用ld作为链接器
set(CMAKE_C_LINK_EXECUTABLE
    "<CMAKE_LINKER> <CMAKE_ASM_LINK_FLAGS> <OBJECTS> -o <TARGET>")
set(CMAKE_ASM_LINK_EXECUTABLE
    "<CMAKE_LINKER> <CMAKE_ASM_LINK_FLAGS> <OBJECTS> -o <TARGET>")
# 编译选项
add_compile_options (-O2 -m32 -Wall -Werror -ffreestanding -nostdinc -fno-builtin)

include_directories (. include)
set(LOADERSRC
    src/head.s
    src/main.c
    src/log.c
    src/elf.c
    src/memory.c
    fs/disk_drv.c
    fs/superblk.c
    fs/nodes.c
    fs/file.c)

add_executable (loader ${LOADERSRC})



