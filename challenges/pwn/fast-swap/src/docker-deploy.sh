#! /bin/sh
#
docker build --network host -t quicker .
docker run --rm -p 1338:1338 --network host quicker
