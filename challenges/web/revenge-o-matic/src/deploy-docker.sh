#! /bin/sh
docker build -t uscg-web-beg-o-matic .
docker run --rm -p 1337:3000 uscg-web-beg-o-matic