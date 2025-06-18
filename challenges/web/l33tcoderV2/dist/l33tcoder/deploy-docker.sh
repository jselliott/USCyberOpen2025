#! /bin/sh
docker build -t uscg-web-leetcoder .
docker run --rm -p 1337:5000 uscg-web-leetcoder