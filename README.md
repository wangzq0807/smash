SMASH
=================
## 简介
smash是一个类Unix的内核，实现了基于分段和分页的内存管理，单个进程最大4GB虚拟地址空间，支持minix-v2l文件系统，实现了写实复制(fork)，管道等机制。

![smash](screenshot.gif)


## 开发环境
* Ubuntu 16.04
* gcc 5.4.0
* bochs 2.6.5+
## 根文件系统
要运行smash，首先需要创建一块虚拟磁盘，具体创建方法如下，你也可以直接[下载](https://pan.baidu.com/s/1w8Xrc3vILAlCCl2TACJGXg)
```sh
# 创建一个10M的空文件
dd if=/dev/zero of=hd.img bs=1M count=10
# 创建一个loop设备
sudo losetup /dev/loop0 hd.img
# 对loop设备进行分区：增加一个主分区,分区号为1
sudo fdisk /dev/loop0
# 刷新loop设备的分区表
sudo partprobe /dev/loop0
# 将第一个分区格式化为minix v2文件系统
sudo mkfs.minix -2 /dev/loop0p1
# 挂载第一个分区
sudo mount /dev/loop0p1 /mnt
# 创建dev和bin目录
sudo mkdir /mnt/dev /mnt/bin
# 创建tty设备节点
sudo mknod /mnt/dev/tty c 1 1
```
## 编译&运行
>如果是下载的根文件系统，可以跳过下面1和2的编译步骤，直接运行.
### 1. 编译用户工具
* 到下面的地址下载源码，然后执行make命令
> `https://github.com/wangzq0807/know_how/tree/prog/program/sub-sys/libc`
* make后会生成bash, ls, cat, echo, rm等可执行文件
* 将这些可执行文件都拷贝到`/mnt/bin`目录（也就是虚拟磁盘的`/bin`目录）
### 2. 编译内核
```sh
sudo umount /mnt
# 将虚拟磁盘拷贝到smash根目录
cp hd.img smash/
cd smash
make
```
### 3. 运行
```sh
bochs -f bochsrc
```
## 系统调用一览
```c
extern int exit(int code);
extern int fork(void);
extern int read(int fd, char *buf, int count);
extern int write(int fd, const char *buf, int count);
extern int open(const char *pathname, int flags, int mode);
extern int close(int fd);
extern int waitpid(int pid, int *status, int options);
extern int creat(const char *pathname, int mode);
extern int link(const char *oldpath, const char *newpath);
extern int unlink(const char *pathname);
extern int execve(const char *pathname, char *const argv[], char *const envp[]);
extern int chdir(const char *pathname);
extern int mkdir(const char *pathname, int mode);
extern int rmdir(const char *pathname);
extern int pause(void);
extern int getpid(void);
extern int pipe(int fd[2]);
extern int dup(int fd);
```
## 参考
* [Linux内核完全剖析:基于0.12内核](https://book.douban.com/subject/3229243/)
* [The Design of the UNIX Operating System](https://book.douban.com/subject/1768601/)
* [wiki.osdev.org](http://wiki.osdev.org/Main_Page)
* [MIT 6.828 xv6](http://pdos.csail.mit.edu/6.828/2011/xv6.html)
* [386BSD](https://github.com/dspinellis/unix-history-repo)
* [INTEL 80386 PROGRAMMER'S REFERENCE MANUAL](https://css.csail.mit.edu/6.858/2014/readings/i386.pdf)
* [minix3](http://www.minix3.org/)
## License
* MIT License