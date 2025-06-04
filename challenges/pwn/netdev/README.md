# Netdev

Players will netcat into the environment, which will spawn the the challenge in qemu and give them unprivileged shell access in qemu.
The environment is a minimal Linux kernel + Busybox userspace.
Once they have shell access they can upload a compiled payload and run the payload to interact with the kernel module.
Since only shell access is granted, they can upload binary files by compressing their payload, and copying base64 output into the shell (or writing a script to do it for them).
Users are also given a C header file to write their client programs to interact with the module via the device at `/proc/netdev`.

The kernel module is a dummy network device which has ioctl operations to configure the device, and read/write opererations to read/write to the ringbuffer.

There is a buffer overread that allows users to leak pointers and defeat KASLR, and a type confusion that gives the user arbitrary kernelspace read/write primitives (with default protections enabled).
Since the user is given a copy of the kernel image and the kernel module, they can run it on their own local qemu instance to get symbol offsets from kallsyms, and decompile the kernel.
There are many ways to get the flag using the read/write primitive.
For instance, one solution could be to locate the current process's `task_struct` and its `cred`, overwrite the suid field, and elevate to root.

## Deployment

Build the container
```bash
cd src
docker build . -t netdev
```

Run the container on port 5000 (you can change the port by changing the first number)
```bash
docker run --rm -p 5000:5000 -it netdev
```

## Connect to the challenge
Connect to the challenge with netcat
```bash
nc localhost 5000
```

You will be greeted with a shell
```
Booting kernel...
Boot took 1.51 seconds

Welcome to SVUSCG
tip: Run "stty -echo"
-sh: can't access tty; job control turned off
~ $
```
