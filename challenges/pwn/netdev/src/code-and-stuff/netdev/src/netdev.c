#include "netdev.h"
#include "ringbuf.h"
#include "util.h"
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

MODULE_LICENSE("GPL");

struct loopback_device {
    struct ring_buffer ringbuf;
};

struct unicast_device {
    struct ring_buffer ringbuf;
    struct ip_address dst;
};

struct broadcast_device {
    struct advertisement advertisement;
};

struct netdev {
    enum netdev_mode mode;
    union {
        struct loopback_device loopback;
        struct unicast_device unicast;
        struct broadcast_device broadcast;
    };
};

static long device_ioctl(struct file *_f, unsigned int cmd, unsigned long arg);
static ssize_t device_read(struct file *f, char *buf, size_t len, loff_t *off);
static ssize_t device_write(struct file *f, const char *buf, size_t len,
                            loff_t *off);

DEFINE_MUTEX(lock);

static struct netdev netdev;

const struct proc_ops proc_ops = {
    .proc_read = device_read,
    .proc_write = device_write,
    .proc_ioctl = device_ioctl,
};

static uint8_t ringbuf_data[2048];

noinline static long set_destination(struct ip_address dst) {
    if (netdev.mode != MODE_UNICAST) {
        return -EPERM;
    }

    switch (dst.version) {
    case IPV4:
        netdev.unicast.dst.version = IPV4;
        memcpy(netdev.unicast.dst.address.ipv4, dst.address.ipv4, 4);
        return 0;
    case IPV6:
        netdev.unicast.dst.version = IPV6;
        memcpy(netdev.unicast.dst.address.ipv6, dst.address.ipv6, 6);
        return 0;
    default:
        return -EINVAL;
    }
}

noinline static long get_destination(struct ip_address __user *dst) {
    if (netdev.mode != MODE_UNICAST) {
        return -EPERM;
    }

    uint16_t version_len = 0;
    long result =
        copy_from_user(&version_len, &dst->version, sizeof(dst->version));
    if (result != 0) {
        return -EFAULT;
    }

    // HACK: Removes the buffer overflow check
    void *buffer = kmalloc(version_len, GFP_KERNEL);
    if (buffer == NULL) {
        return -ENOMEM;
    }

    memcpy(buffer, &netdev.unicast.dst.address, version_len);
    result = copy_to_user(&dst->address, buffer, version_len);
    kfree(buffer);
    if (result != 0) {
        return -EFAULT;
    }

    return 0;
}

noinline static long send_advertisement(struct advertisement __user *ad) {
    if (netdev.mode != MODE_BROADCAST) {
        return -EPERM;
    }

    struct advertisement advertisement;
    long result = copy_from_user(&advertisement, ad, sizeof(advertisement));
    if (result != 0) {
        return -EFAULT;
    }

    netdev.broadcast.advertisement = advertisement;
    return 0;
};

static long locked_ioctl(unsigned int cmd, unsigned long arg) {
    switch (cmd) {
    case NETDEV_CHANGE_MODE:
        netdev.mode = arg;
        return 0;
    case NETDEV_SET_DESTINATION:
        return set_destination(*(struct ip_address *)&arg);
    case NETDEV_GET_DESTINATION:
        return get_destination((struct ip_address *)arg);
    case NETDEV_SEND_ADVERTISEMENT:
        return send_advertisement((struct advertisement *)arg);
    default:
        return -EINVAL;
    }
}

static long device_ioctl(struct file *_f, unsigned int cmd, unsigned long arg) {
    mutex_lock(&lock);
    long result = locked_ioctl(cmd, arg);
    mutex_unlock(&lock);
    return result;
}

static ssize_t locked_read(char __user *buf, size_t len) {
    if ((netdev.mode != MODE_LOOPBACK) && (netdev.mode != MODE_UNICAST)) {
        return -EPERM;
    }

    char *buffer = kmalloc(len, GFP_KERNEL);
    if (buffer == NULL) {
        return -ENOMEM;
    }

    size_t size = ringbuf_read(&netdev.loopback.ringbuf, buffer, len);
    long result = copy_to_user(buf, buffer, size);
    kfree(buffer);
    if (result != 0) {
        return -EFAULT;
    }

    return size;
}

static ssize_t device_read(struct file *f, char *buf, size_t len, loff_t *off) {
    mutex_lock(&lock);
    ssize_t result = locked_read(buf, len);
    mutex_unlock(&lock);
    return result;
}

static ssize_t locked_write(const char __user *buf, size_t len) {
    if ((netdev.mode != MODE_LOOPBACK) && (netdev.mode != MODE_UNICAST)) {
        return -EPERM;
    }

    char *buffer = kmalloc(len, GFP_KERNEL);
    if (buffer == NULL) {
        return -ENOMEM;
    }

    long result = copy_from_user(buffer, buf, len);
    if (result != 0) {
        kfree(buffer);
        return -EFAULT;
    }

    size_t size = ringbuf_write(&netdev.loopback.ringbuf, buffer, len);
    kfree(buffer);
    return size;
}

static ssize_t device_write(struct file *f, const char *buf, size_t len,
                            loff_t *off) {
    mutex_lock(&lock);
    ssize_t result = locked_write(buf, len);
    mutex_unlock(&lock);
    return result;
}

int init_module(void) {
    netdev.mode = MODE_LOOPBACK;
    netdev.loopback.ringbuf.head = 0;
    netdev.loopback.ringbuf.tail = 0;
    netdev.loopback.ringbuf.cap = sizeof(ringbuf_data);
    netdev.loopback.ringbuf.ptr = ringbuf_data;

    struct proc_dir_entry *proc_entry =
        proc_create("netdev", 0666, NULL, &proc_ops);
    BUG_ON(proc_entry == NULL);
    return 0;
}
