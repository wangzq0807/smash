CC = gcc
LD = ld
CFLAGS = -g -O2  -m32 -Wall -Werror -nostdinc -fno-builtin -fno-stack-protector
LDFLAGS = -m elf_i386 -static
INCLUDES = -I ./ -I ./include
BUILD_PATH = _build

CRT = usr/crt.o usr/syscalls.o usr/print.o
BASH_OBJS = bash.o $(CRT)
LS_OBJS = ls.o $(CRT)
HELLO_OBJS = helloworld.o $(CRT)
ECHO_OBJS = echo.o $(CRT)
LN_OBJS = ln.o $(CRT)
CAT_OBJS = cat.o $(CRT)
RM_OBJS = rm.o $(CRT)
PIPECHO_OBJS = pipecho.o $(CRT)

OBJECTSS = $(BASH_OBJS) $(LS_OBJS) $(HELLO_OBJS) $(ECHO_OBJS) $(LN_OBJS) $(CAT_OBJS) $(RM_OBJS) $(PIPECHO_OBJS)
EXES = bash ls helloworld echo ln cat rm pipecho
# 自动生成depends文件名
DEPS = $(OBJECTSS:.o=.d)

all: $(EXES)
	@objdump -S bash > bash.sym

bash: $(BASH_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

ls: $(LS_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

helloworld: $(HELLO_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

echo: $(ECHO_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

ln: $(LN_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

cat: $(CAT_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

rm: $(RM_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

pipecho: $(PIPECHO_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

# 源文件编译规则
%.o : %.c
	$(CC) $(CFLAGS) -MP -MMD -c $< -o $@ $(INCLUDES)

%.o : %.S
	$(CC) $(CFLAGS) -MP -MMD -c $< -o $@ $(INCLUDES)

# 包含所有依赖
-include $(DEPS)

clean:
	-rm $(OBJECTSS) $(DEPS) $(EXES) bash.sym
