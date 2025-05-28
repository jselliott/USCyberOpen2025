#!/bin/bash

env -i \
    PATH="/usr/bin:/bin" \
    HOME="/tmp" \
    USER="chal" \
    LANG="C" \
    LC_ALL="C" \
    ./decryptonite_server "$@"

