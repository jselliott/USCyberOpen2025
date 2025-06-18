FROM ubuntu:16.04 as builder

WORKDIR /build
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get -y install build-essential libssl-dev
COPY . .
RUN make


FROM ubuntu:16.04

RUN useradd --system chal
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y openssl
WORKDIR /chal

COPY --from=builder /build/decryptonite_server .
COPY --from=builder /build/decryptonite_client .
COPY flag.txt /
COPY start_server.sh ./

# Ensure chal dir is read only
RUN chown -R root:root ./* && \
    chmod -R o-w ./*

EXPOSE 8888

CMD ["su", "chal", "-c", "./start_server.sh"]
