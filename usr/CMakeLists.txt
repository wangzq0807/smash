
set (LD_FLAGS "-m elf_i386 -static")
set(CMAKE_C_LINK_EXECUTABLE
    "ld ${LD_FLAGS} <OBJECTS> -o <TARGET>")

set (CORE_SRCS
    core/crt.c
    core/print.c
    core/syscalls.S)

add_executable (bash bash.c ${CORE_SRCS})
add_executable (ls ls.c ${CORE_SRCS})
add_executable (helloworld helloworld.c ${CORE_SRCS})
add_executable (echo echo.c ${CORE_SRCS})
add_executable (ln ln.c ${CORE_SRCS})
add_executable (cat cat.c ${CORE_SRCS})
add_executable (rm rm.c ${CORE_SRCS})
add_executable (pipecho pipecho.c ${CORE_SRCS})

