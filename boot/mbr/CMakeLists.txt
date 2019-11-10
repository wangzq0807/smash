include(CMakeASMInformation)
 
set(CMAKE_ASM_LINK_FLAGS "-m32 -nostartfiles -Ttext 0x7c00")

# 编译选项
add_compile_options (-g -O2 -m32 -Wall -Werror -ffreestanding -nostdinc -fno-builtin)
 
add_executable (mbr mbr.s)
