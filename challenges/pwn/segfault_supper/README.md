# Segfault Supper

A binary that implements a food ordering service where all database transactions happen in a background thread. Players must exploit a race condition to order more than three items, which can be used once to leak an address and break ASLR, then again to corrupt the return address and return to the order menu where they can order the flag when the stack is set up such that is_admin is nonzero.

### Deployment Instructions

Run src/deploy-docker.sh to build and deploy the server. Default port is 1337, you can change the mapping in the script if you want.

### Testing Instructions

Run solution/solve.py:
```
❯ ./solve.py --help
usage: solve.py [-h] [-t TARGET] [-p PORT] [--timeout TIMEOUT]

Exploit for Segfault Supper. Try increasing --timeout if it doesn't work consistently.

options:
  -h, --help           show this help message and exit
  -t, --target TARGET  ip of target
  -p, --port PORT      port of target
  --timeout TIMEOUT    timeout in seconds for some operations
```
