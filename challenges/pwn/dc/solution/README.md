# CVE-2024-61..

## Solution

To solve this challenge the user needs to be able to find vulnerabilities within source code. As the title and hint suggest this code is from a CVE and from a router. If the author's name is known the CVE's can be easily looked up for hints, or looking through the 100 or so CVE's can help narrow down where to look within the code.

The code flow has been modified to support reading from stdin/out instead of the normal socket, following the input flow of the data there are at least 3 vulnerabilities that can be found.

The first is within the initial readloop or 'uh_tcp_recv_header', the while loop will continue to read until the string '\r\n\r\n' is read, or the size read is greater than (UH_LIMIT_MSGHEAD/10), roughly 13000 bytes. The vulnerability is that the len variable is never correctly incremented and will always read UH_LIMIT_MSGHEAD characters into the buffer of a fixed size. The downside to this, is the user will need to supply ~144000 bytes, but otherwise gives a direct buffer overflow onto the stack.

The second vulnerabilty, is of a similar means and is within reading of files, from post requests. As the same while loop mechanism is utilized, results in a similar bug and vulnerability.

The third vulnerability, and the point of attack for `exploit.py` is to target a buffer overflow within an sprintf of `uh_http_header_recv`. The code does not correctly bounds check the `boundry` field from file posts. This allows for an overflow into a stack buffer from a 128 length buffer. Making this an easier target than the 144000 buffer from before.

Using this a simple jump to the stack allows for shellcode to be executed, as this program is using qemu-static-mips the memory regions are reasonable deterministic this can be an easy feat. But during testing it was found the stack can still range from several locations, so a simple rop chain was used to pivot to the static in a more reliable way. Then the flag can be read from /flag.txt.
