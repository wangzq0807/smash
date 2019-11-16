# SMASH

## 简介

smash是一个类Unix的内核，实现了基于分段和分页的内存管理，单个进程最大4GB虚拟地址空间，支持minix-v2l文件系统，实现了写实复制(fork)，管道等机制。

![smash](screenshot.gif)

## 开发环境

* Ubuntu 18.04
* gcc 7.4.0
* bochs-x 2.6.5+
* cmake 3.0+

## 编译内核

```sh
cd smash/_build
cmake ../
make
```

### 运行
在_build目录下执行

```sh
make run
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

## 一些细节

### 目录结构

├── boot<br>
│   ├── loader<br>
│   └── mbr<br>
├── _build<br>
├── include<br>
├── kernel<br>
├── tools<br>
└── usr<br>

* boot : 存放引导程序
* kernel : 内核代码
* include : unix标准头文件
* usr : 应用程序

### kernel设计

### CMake

* 使用config.h.in来生成配置选项

### GCC

* 不使用标准库的头文件(用自定义的): -nostdinc

### Bochs

* IO debugger : 在bochsrc中添加"port_e9_hack: enabled=1"
* GUI Debug : 安装libgtk2.0-dev, 然后添加编译选项--enable-debugger --enable-disasm --enable-debugger-gui,编译bochs. 然后在bochsrc中添加"display_library: x, options="gui_debug""
