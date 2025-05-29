#! /bin/sh
#
docker build --network host -t multi-target .
docker run --rm -p 1341:1341 --network host multi-target
