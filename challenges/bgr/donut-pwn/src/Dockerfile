FROM ubuntu:22.04

RUN apt-get update && apt-get install -y socat

WORKDIR /app

COPY flag.txt .
COPY donut .

RUN chmod +x donut

ENTRYPOINT ["socat","TCP-LISTEN:1337,reuseaddr,fork","EXEC:\"./donut\""]