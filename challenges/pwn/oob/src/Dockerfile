FROM debian:bookworm-slim

RUN apt-get update && \
    apt-get install -y socat

# Create a non-root user for running the challenge
RUN useradd -m ctf

# Copy challenge files
COPY ./chall /chall
RUN chmod +x /chall/chall.bin
RUN chmod +x /chall/libc.so.6
RUN chmod +x /chall/ld-linux-x86-64.so.2

RUN mv /chall/flag.txt /
RUN chmod 444 /flag.txt

# Set ownership to non-root user
RUN chown -R ctf:ctf /home/ctf

# Switch to non-root user
USER ctf

# Expose port
EXPOSE 1340

WORKDIR /chall

# Use socat to bind port 1337 to your challenge
CMD socat TCP-LISTEN:1340,reuseaddr,fork EXEC:"timeout 60 /chall/chall.bin"

