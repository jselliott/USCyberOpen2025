#include "filter_vm.h"
#include "util.h"
#include <linux/ioctl.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

#define MAGIC 'f'
#define PROGRAM _IOW(MAGIC, 1, struct program *)
#define EXECUTE _IOW(MAGIC, 2, char *)

MODULE_LICENSE("GPL");

struct program {
    uint8_t *code;
    size_t len;
};

static struct program current_program = {
    .code = NULL,
    .len = 0,
};

struct thread_args {
    char *filename;
    struct completion *done;
};

noinline static int should_drop_inner(void *args) {
    char *filename = ((struct thread_args *)args)->filename;
    struct completion *done = ((struct thread_args *)args)->done;
    struct filter_packet packet = {0};
    int result;

    // Open file
    struct file *f = filp_open(filename, O_RDONLY, 0);
    if (IS_ERR(f)) {
        result = (int)(uint64_t)f;
        goto out;
    }

    // Read file
    loff_t offset = 0;
    ssize_t packet_size =
        kernel_read(f, packet.mem, sizeof(struct filter_packet), &offset);
    filp_close(f, NULL);

    if (packet_size < 0) {
        result = -EIO;
        goto out;
    }

    result = execute_filter(packet.mem, sizeof(struct filter_packet),
                            current_program.code, current_program.len);

out:
    complete(done);
    return result;
}

noinline static int spawn_filter_thread(char *filename) {
    struct completion done;
    init_completion(&done);

    struct thread_args args = {.filename = filename, .done = &done};
    struct task_struct *task =
        kthread_run(should_drop_inner, &args, "should_drop_inner");

    if (IS_ERR(task)) {
        return (int)(uint64_t)task;
    }

    wait_for_completion(&done);
    int result = kthread_stop(task);
    return result;
}

noinline static long should_drop(char __user *filename) {
    char buf[256] = {0};

    // Check if a program exists
    if (current_program.code == NULL) {
        return -EINVAL;
    }

    // Get filename from user
    long result = strncpy_from_user(buf, filename, sizeof(buf));
    if (result < 0) {
        return -EFAULT;
    }

    result = spawn_filter_thread(buf);

    return result;
}

noinline static long set_program(struct program __user *user_program) {
    // Get the struct from the user
    struct program p;
    long result = copy_from_user(&p, user_program, sizeof(struct program));
    if (result != 0) {
        return -EFAULT;
    }

    if (p.len > 0x1000) {
        return -EINVAL;
    }

    // Allocate the code
    uint8_t *code = kmalloc(p.len, GFP_KERNEL);
    if (code == NULL) {
        return -ENOMEM;
    }

    // Copy the code from the user
    result = copy_from_user(code, p.code, p.len);
    if (result != 0) {
        kfree(code);
        return -EFAULT;
    }

    if (current_program.code != NULL) {
        kfree(current_program.code);
    }

    current_program.code = code;
    current_program.len = p.len;
    return 0;
}

static long device_ioctl(struct file *_f, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
    case PROGRAM:
        return set_program((struct program *)arg);
    case EXECUTE:
        return should_drop((char *)arg);
    default:
        return -EINVAL;
    }
}

const struct proc_ops proc_ops = {
    .proc_ioctl = device_ioctl,
};

int init_module(void) {
    struct proc_dir_entry *proc_entry =
        proc_create("filter", 0666, NULL, &proc_ops);
    BUG_ON(proc_entry == NULL);
    return 0;
}

void cleanup_module(void) {}
