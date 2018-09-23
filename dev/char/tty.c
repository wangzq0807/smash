#include "tty.h"
#include "console.h"
#include "log.h"

#define QUEUE_LEN    256
typedef struct _ttyDev      ttyDev;
typedef struct _ttyQueue    ttyQueue;

struct _ttyQueue
{
    int     tq_head;
    int     tq_tail;
    char    tq_buf[QUEUE_LEN];
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
    console_tty.td_write_q.tq_head = 0;
    console_tty.td_write_q.tq_tail = 0;
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

static int
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
    char sz[2] = {c, 0};
    if (c == '\b') {
        int bs = _backspace_queue(&console_tty.td_read_q);
        if (bs == 0) {
            console_print(sz);
        }
    }
    else if(c == '\r') {
        _put_queue(&console_tty.td_read_q, c);
        console_print(sz);
    }
    else {
        _put_queue(&console_tty.td_read_q, c);
        console_print(sz);
    }
}

int
tty_read(char *buf, int cnt)
{
    int i = 0;
    while(i++ < cnt) {
        buf[i] = (char)_pop_queue(&console_tty.td_read_q);
    }
    return 0;
}

int
tty_write(const char *buf, int cnt)
{
    int i = 0;
    while(i++ < cnt)
        _put_queue(&console_tty.td_write_q, buf[i]);
    return 0;
}
