FROM archlinux

COPY build.sh /opt/build.sh

RUN \
    pacman -Sy && \
    pacman -S --noconfirm wget base-devel flex bison cpio qemu-system-aarch64 aarch64-linux-gnu-gcc bc python3 musl-aarch64 && \
    useradd -m ctf && \
    chmod +x /opt/build.sh

USER ctf
WORKDIR /home/ctf
RUN /opt/build.sh
