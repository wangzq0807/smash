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
add_custom_target (hd.img ALL)
add_custom_command (TARGET hd.img PRE_BUILD
    COMMAND objcopy -j .text -O binary boot/boot boot.img
    COMMAND objcopy -O binary kernel/smash smash.img
    COMMAND objdump -S kernel/smash > smash.sym
	COMMAND dd if=boot.img of=hd.img bs=432 count=1 conv=notrunc
    COMMAND dd if=/dev/zero of=hd.img bs=512 count=120 seek=1 conv=notrunc
    COMMAND dd if=smash.img of=hd.img bs=512 count=120 seek=1 conv=notrunc)
add_dependencies (hd.img smash boot)