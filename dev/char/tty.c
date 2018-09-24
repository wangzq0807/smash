#include "tty.h"
#include "console.h"
#include "log.h"
#include "arch/task.h"
#include "asm.h"
#include "string.h"

#define QUEUE_LEN    256
typedef struct _ttyDev      ttyDev;
typedef struct _ttyQueue    ttyQueue;

struct _ttyQueue
{
    int     tq_head;
    int     tq_tail;
    char    tq_buf[QUEUE_LEN];
    Task    *tq_wait_task;
};

struct _ttyDev
{
    ttyQueue    td_write_q;
    ttyQueue    td_read_q;
};

ttyDev  console_tty;

int
init_tty()
{
    console_tty.td_read_q.tq_head = 0;
    console_tty.td_read_q.tq_tail = 0;
    console_tty.td_read_q.tq_wait_task = NULL;
    console_tty.td_write_q.tq_head = 0;
    console_tty.td_write_q.tq_tail = 0;
    console_tty.td_write_q.tq_wait_task = NULL;
    return 0;
}

static int
_is_empty_queue(ttyQueue *queue)
{
    if (queue->tq_head == queue->tq_tail)
        return 1;
    else
        return 0;
}

// TODO: 不要static，会导致GCC出BUG
int
_is_full_queue(ttyQueue *queue)
{
    if ((queue->tq_head + 1) == queue->tq_tail) {
        return 1;
    }
    // TODO : 下面这种判断方式会导致GCC出BUG
    else if ((queue->tq_tail == 0) && (queue->tq_head == (QUEUE_LEN-1))) {
        return 1;
    }
    else {
        return 0;
    }
}

static int
_put_queue(ttyQueue *queue, char c)
{
    if (_is_full_queue(queue))  return -1;
    queue->tq_buf[queue->tq_head] = c;
    queue->tq_head++;
    if (queue->tq_head == QUEUE_LEN)
        queue->tq_head = 0;
    return 0;
}

static int
_pop_queue(ttyQueue *queue)
{
    if (_is_empty_queue(queue)) return -1;
    int ret = queue->tq_buf[queue->tq_tail];
    if (++queue->tq_tail == QUEUE_LEN)
        queue->tq_tail = 0;
    return ret;
}

// TODO: 不要static，会导致GCC出BUG
int
_backspace_queue(ttyQueue *queue)
{
    if (_is_empty_queue(queue)) return -1;
    int prehead = queue->tq_head - 1;
    if (prehead == -1)
        prehead = QUEUE_LEN - 1;
    char lc = queue->tq_buf[prehead];
    if (lc == '\r') return -1;

    queue->tq_head = prehead;
    return 0;
}

void
on_tty_intr(char c)
{
    return ;
    char sz[2] = {c, 0};
    if (c == '\b') {
        int bs = _backspace_queue(&console_tty.td_read_q);
        if (bs == 0) {
            console_print(sz);
        }
    }
    else if(c == '\r') {
        if (_is_full_queue(&console_tty.td_read_q))  return;
        _put_queue(&console_tty.td_read_q, c);
        wakeup(console_tty.td_read_q.tq_wait_task);
        console_print(sz);
    }
    else {
        if (_is_full_queue(&console_tty.td_read_q))  return;
        _put_queue(&console_tty.td_read_q, c);
        wakeup(console_tty.td_read_q.tq_wait_task);
        console_print(sz);
    }
}

int
tty_read(char *buf, int cnt)
{
    return 0;
    int i = 0;
    if (_is_empty_queue(&console_tty.td_read_q)) {
        console_tty.td_read_q.tq_wait_task = current_task();
        sleep(console_tty.td_read_q.tq_wait_task);
    }

    while(i < cnt) {
        if (_is_empty_queue(&console_tty.td_read_q)) {
            console_tty.td_read_q.tq_wait_task = current_task();
            sleep(console_tty.td_read_q.tq_wait_task);
        }

        buf[i] = (char)_pop_queue(&console_tty.td_read_q);
        if (buf[i] == '\r')
            break;
        ++i;
    }
    return i;
}

int
tty_write(const char *buf, int cnt)
{
    console_print(buf);
    // int i = 0;
    // while(i++ < cnt)
    //     _put_queue(&console_tty.td_write_q, buf[i]);
    return strlen(buf);
}
