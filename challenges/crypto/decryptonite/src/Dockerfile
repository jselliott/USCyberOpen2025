FROM ubuntu:16.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get -y install build-essential libssl-dev gdbserver
RUN apt-get -qq update && \
    apt-get -qq install -y --no-install-recommends libssl-dev
RUN useradd --system chal

WORKDIR /chal

COPY flag.txt /
COPY start_server.sh ./
COPY decryptonite_server ./

# Ensure chal dir is read only
RUN chown -R root:root ./* && \
    chmod -R o-w ./*

EXPOSE 8888

CMD ["su", "chal", "-c", "./start_server.sh"]
