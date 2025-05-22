#!/bin/bash

echo "If this doesn't work make sure to cd to the src directory before running"
docker build . -t segfault_supper && \
docker run --rm --name segfault_supper -p 1337:1337 -d segfault_supper

