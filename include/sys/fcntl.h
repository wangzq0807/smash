#ifndef __FCNTL_H__
#define __FCNTL_H__

#define O_RDONLY    0x1
#define O_WRONLY    0x2
#define O_NONBLOCK  0x4
#define O_APPEND    0x8

#define O_CREAT     0x200
#define O_TRUNC     0x400

#define O_RDWR      (O_RDONLY|O_WRONLY)
#endif // __FCNTL_H__
