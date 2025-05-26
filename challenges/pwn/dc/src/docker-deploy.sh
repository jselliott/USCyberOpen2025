#! /bin/sh
#
docker build --network host -t cve-2024-61 .
docker run --rm -p 1337:1337 --network host cve-2024-61
