docker build -t uscg-open-web-versioned . && \
docker run -p 1337:80 --rm uscg-open-web-versioned