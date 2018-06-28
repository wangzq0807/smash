CC = gcc
LD = ld
CFLAGS = -g -O2  -m32 -Wall -Werror -ffreestanding -nostdinc
LDFLAGS = -m elf_i386
INCLUDES = -I ./ -I ./include
BUILD_PATH = _build

# 汇编源文件
ASMSRCS = arch/head.S \
		  arch/intr.S
# 查找所有的.c文件
SOURCES = $(shell find ./ -name '*.c' -printf '%T@\t%p\n' \
			| sort -k 1nr | cut -f2-)
# 自动生成的obj文件名, head.o需放在$(OBJECTS)第一位
OBJECTS = $(patsubst %.S, $(BUILD_PATH)/%.o, $(ASMSRCS))
OBJECTS += $(patsubst %.c, $(BUILD_PATH)/%.o, $(SOURCES))
# 自动生成depends文件名
DEPS = $(OBJECTS:.o=.d)

all : dirs
	@$(MAKE) hd.img --no-print-directory
# NOTE: kernel.sym便于调试
# boot只需要代码区
# NOTE:kernel除了代码区以外还需要数据区和bss区等
hd.img : boot kernel
	@objcopy -j .text -O binary boot boot.img
	@objcopy -O binary kernel kernel.img
	@objdump -S kernel > kernel.sym
	@echo "将内核写入虚拟磁盘hd.img..."
	@dd if=boot.img of=hd.img bs=432 count=1 conv=notrunc
	@dd if=/dev/zero of=hd.img bs=512 count=120 seek=1 conv=notrunc
	@dd if=kernel.img of=hd.img bs=512 count=120 seek=1 conv=notrunc

# 引导程序的链接规则
boot : $(BUILD_PATH)/arch/boot.o
	$(LD) $(LDFLAGS) -Ttext 0x00 $^ -o boot

# 内核的链接规则
kernel : $(OBJECTS)
	$(LD) $(LDFLAGS) -Ttext 0x00 $^ -o kernel

# 源文件编译规则
$(BUILD_PATH)/%.o : %.c
	$(CC) $(CFLAGS) -MP -MMD -c $< -o $@ $(INCLUDES)

$(BUILD_PATH)/%.o : %.S
	$(CC) $(CFLAGS) -c $^ -o $@

# 包含所有依赖
-include $(DEPS)

.PHONY: dirs
dirs:
	@echo "创建编译目录..."
	@mkdir -p $(dir $(OBJECTS))
	@mkdir -p $(BUILD_PATH)

.PHONY:
clean:
	@$(RM) -rf boot.img kernel.img kernel.sym kernel boot $(BUILD_PATH)/*
