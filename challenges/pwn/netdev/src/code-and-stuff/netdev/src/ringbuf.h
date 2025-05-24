#include <linux/types.h>

struct ring_buffer {
    size_t head;
    size_t tail;
    size_t cap;
    uint8_t *ptr;
};

// Returns bytes read
size_t ringbuf_read(struct ring_buffer *ringbuf, uint8_t *data, size_t len);

// Returns bytes written
size_t ringbuf_write(struct ring_buffer *ringbuf, const uint8_t *data,
                     size_t len);
