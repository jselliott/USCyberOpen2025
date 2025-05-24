#include "ringbuf.h"
#include "util.h"
#include <linux/errno.h>
#include <linux/slab.h>

noinline size_t ringbuf_read(struct ring_buffer *ringbuf, uint8_t *data,
                             size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (ringbuf->head == ringbuf->tail) {
            return i;
        }

        data[i] = ringbuf->ptr[ringbuf->head];
        ringbuf->head++;
        ringbuf->head %= ringbuf->cap;
    }

    return len;
}

noinline size_t ringbuf_write(struct ring_buffer *ringbuf, const uint8_t *data,
                              size_t len) {
    for (size_t i = 0; i < len; i++) {
        ringbuf->ptr[ringbuf->tail] = data[i];
        ringbuf->tail++;
        ringbuf->tail %= ringbuf->cap;
    }

    return len;
}
