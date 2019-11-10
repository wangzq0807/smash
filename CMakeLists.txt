cmake_minimum_required (VERSION 3.0)

include_directories (common include)

add_subdirectory(boot)
add_subdirectory(kernel)
add_subdirectory(usr)
#add_subdirectory(mytest)

# 创建镜像
# boot只需要代码区
# NOTE:kernel除了代码区以外还需要数据区和bss区等
# NOTE:写入的数据不能超过xxKB
