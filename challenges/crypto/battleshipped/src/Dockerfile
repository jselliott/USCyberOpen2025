FROM python:3-alpine

RUN apk add socat

WORKDIR /app

COPY main.py .
COPY flag.txt /flag.txt

ENTRYPOINT ["socat","TCP-LISTEN:1337,reuseaddr,fork","EXEC:\"python3 -u main.py\""]