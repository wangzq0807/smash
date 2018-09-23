#ifndef __TTY_H__
#define __TTY_H__

int
init_tty();

void
on_tty_intr(char c);

int
tty_read(char *buf, int cnt);

int
tty_write(const char *buf, int cnt);

#endif // __TTY_H__
