#!/bin/bash

echo "If this doesn't work make sure to cd to the src directory before running"
docker build . -t decryptonite && \
docker run --rm --name decryptonite --privileged -p 8888:8888 -d decryptonite

