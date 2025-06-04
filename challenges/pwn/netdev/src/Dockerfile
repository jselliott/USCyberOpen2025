FROM alpine

RUN \
    apk add qemu-system-aarch64 socat && \
    adduser -D ctf

COPY Image.gz /opt/Image.gz
COPY initramfs.cpio.gz /opt/initramfs.cpio.gz
COPY run.sh /opt/run.sh

WORKDIR "/home/ctf"
EXPOSE 5000
USER "ctf"

CMD ["socat", "-dd", "TCP4-LISTEN:5000,reuseaddr,fork", "EXEC:'/opt/run.sh',pty,stderr"]
