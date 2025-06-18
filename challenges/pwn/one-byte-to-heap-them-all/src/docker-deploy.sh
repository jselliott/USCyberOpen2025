#! /bin/sh
#
docker build --network host -t heap .
docker run --rm -p 1338:1338 --network host heap
