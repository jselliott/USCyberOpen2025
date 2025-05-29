docker build -t uscg-web-repository . && \
docker run --rm -p 1337:8080 uscg-web-repository