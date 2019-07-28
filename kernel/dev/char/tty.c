#include "tty.h"
#include "console.h"
#include "log.h"
#include "arch/task.h"
#include "asm.h"
#include "string.h"

#define QUEUE_LOG_LEN   3
#define QUEUE_LEN       (1 << QUEUE_LOG_LEN)
typedef struct _ttyDev      ttyDev;
typedef struct _ttyQueue    ttyQueue;

struct _ttyQueue
{
    uint32_t    tq_head;
    uint32_t    tq_tail;
    char        tq_buf[QUEUE_LEN];
    Task        *tq_wait_task;
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

static inline uint32_t
_get_buf_index(uint32_t pos)
{
    return pos & (QUEUE_LEN-1);
}

static inline int
_is_empty_queue(ttyQueue *queue)
{
    return (queue->tq_head == queue->tq_tail);
}

static inline int
_is_full_queue(ttyQueue *queue)
{
    return (queue->tq_head - queue->tq_tail) >= QUEUE_LEN;
}

static int
_put_queue(ttyQueue *queue, char c)
{
    if (_is_full_queue(queue))  return -1;
    const uint32_t pos = _get_buf_index(queue->tq_head);
    queue->tq_buf[pos] = c;
    queue->tq_head++;
    return 0;
}

static int
_pop_queue(ttyQueue *queue)
{
    if (_is_empty_queue(queue)) return -1;
    const uint32_t pos = _get_buf_index(queue->tq_tail);
    const int ret = queue->tq_buf[pos];
    queue->tq_tail++;
    return ret;
}

static int
_backspace_queue(ttyQueue *queue)
{
    if (_is_empty_queue(queue)) return -1;
    const uint32_t prehead = queue->tq_head - 1;
    const uint32_t pos = _get_buf_index(prehead);
    char lc = queue->tq_buf[pos];
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
        if (_is_full_queue(&console_tty.td_read_q))  return;
        _put_queue(&console_tty.td_read_q, c);
        wakeup(console_tty.td_read_q.tq_wait_task);
        console_print(sz);
    }
    else {
        if (_is_full_queue(&console_tty.td_read_q))  return;
        _put_queue(&console_tty.td_read_q, c);
        console_print(sz);
    }
}

int
tty_read(char *buf, int cnt)
{
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
    return i + 1;
}

int
tty_write(const char *buf, int cnt)
{
    console_print(buf);

    return strlen(buf);
}
