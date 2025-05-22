#!/bin/sh

exec 2>&1

# dir
cd /chal

# timeout after 40 sec (process already has a 30s alarm but just in case)
#                             <---| don't touch anything left
#                                 | unless you need a longer timeout
timeout -k1 40 stdbuf -i0 -o0 -e0 ./segfault_supper
#           |^ |^^^^^^^^^^^^^^^^^
#           |  + disable buffering
#           + 40s timeout
