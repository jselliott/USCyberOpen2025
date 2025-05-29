#! /bin/sh
#
docker build --network host -t fgaslr .
docker run --rm -p 1337:1337 --network host fgaslr
