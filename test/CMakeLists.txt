
add_compile_options(-g -O2)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -m32")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -m32")

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

include_directories (../kernel ../kernel/include)
add_subdirectory(container)
