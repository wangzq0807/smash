#ifndef __RING_H__
#define __RING_H__
#include "stdint.h"
#include "string.h"

typedef struct {
    uint8_t     *rb_buf;
    size_t      rb_size;
    size_t      rb_head;
    size_t      rb_tail;
} RingBuf;

static inline RingBuf*
ringbuf_init(RingBuf *ri, uint8_t *bytes, size_t size) {
    ri->rb_buf = bytes;
    ri->rb_size = size;
    ri->rb_head = ri->rb_tail = 0;
    return ri;
}

static inline int
ringbuf_is_full(const RingBuf *ri) {
    return (ri->rb_tail - ri->rb_head) >= ri->rb_size;
}

static inline int
ringbuf_is_empty(const RingBuf *ri) {
    return ri->rb_tail == ri->rb_head;
}

static inline size_t
ringbuf_available(const RingBuf *ri) {
    return ri->rb_tail - ri->rb_head;
}

static inline size_t
ringbuf_put(RingBuf *ri, const uint8_t *bytes, size_t num) {
    size_t ret = 0;
    while (!ringbuf_is_full(ri) && ret < num) {
        size_t pos = ri->rb_tail % ri->rb_size;
        ri->rb_buf[pos] = bytes[ret];
        ret++;
        ri->rb_tail++;
    }
    return ret;
}

static inline size_t
ringbuf_get(RingBuf *ri, uint8_t *bytes, size_t num) {
    size_t ret = 0;
    while (!ringbuf_is_empty(ri) && ret < num) {
        size_t pos = ri->rb_head % ri->rb_size;
        bytes[ret] = ri->rb_buf[pos];
        ret++;
        ri->rb_head++;
    }
    return ret;
}

#endif // __RING_H__
