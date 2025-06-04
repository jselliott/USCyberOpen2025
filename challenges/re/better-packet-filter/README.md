# Better Packet Filter

Players will netcat into the environment, which will spawn the the challenge in qemu and give them unprivileged shell access in qemu.
The environment is a minimal Linux kernel + Busybox userspace.
Once they have shell access they can upload a compiled payload and run the payload to interact with the kernel module.
Since only shell access is granted, they can upload binary files by compressing their payload, and copying base64 output into the shell (or writing a script to do it for them).
They will interact with the module via the device at `/proc/filter`.

The kernel module is a simplified parody of the Berkeley Packet Filter, which is used to improve performance of packet filtering for programs such as tcpdump.
Modern eBPF can be either JIT'ed or interpreted. This challenge interprets programs.

Players can provide a program to be interpreted, which can have one of two return values: ACCEPT or DROP.
The module reads the packet out of a user-provided filename and provides it as input to the interpreter, allowing users to leak the flag via the ACCEPT or DROP return values.

## Deployment

Build the container
```bash
cd src
docker build . -t better-packet-filter
```

Run the container on port 5000 (you can change the port by changing the first number)
```bash
docker run --rm -p 5000:5000 -it better-packet-filter
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
