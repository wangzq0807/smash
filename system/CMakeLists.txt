enable_language(ASM)
# 编译选项
set(CMAKE_C_FLAGS "-g -O2 -m32 -Wall -Werror -ffreestanding -nostdinc -fno-builtin -nostdlib")
set(CMAKE_C_LINK_FLAGS "-m32")
set(CMAKE_ASM_FLAGS "-m32")
include_directories(. ../kernel/include ../include)

set (LIBC_SRCS
    libc/crt.c
    libc/print.c
    libc/syscalls.S)
add_library(crt STATIC ${LIBC_SRCS})
link_libraries(crt)

add_executable  (bash   usr/bash.c)
add_dependencies(bash   crt)
add_executable  (ls     usr/ls.c)
add_dependencies(ls     crt)
add_executable  (helloworld     usr/helloworld.c)
add_dependencies(helloworld     crt)
add_executable  (echo   usr/echo.c)
add_dependencies(echo   crt)
add_executable  (ln     usr/ln.c)
add_dependencies(ln     crt)
add_executable  (cat    usr/cat.c)
add_dependencies(cat    crt)
add_executable  (rm     usr/rm.c)
add_dependencies(rm     crt)
add_executable  (pipecho usr/pipecho.c)
add_dependencies(pipecho crt)

