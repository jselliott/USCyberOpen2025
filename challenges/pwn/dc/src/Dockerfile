FROM debian:bookworm-slim

RUN apt-get update && \
    apt-get install -y socat

# Create a non-root user for running the challenge
RUN useradd -m ctf

# Copy challenge files
COPY ./chall /chall
RUN chmod +x /chall/run.sh 
RUN chmod +x /chall/binary 
RUN chmod +x /chall/qemu-mips
RUN chmod +x /chall/lib/libc.so.6
RUN chmod +x /chall/lib/ld.so.1

RUN mv /chall/flag.txt /
RUN chmod 444 /flag.txt

# Set ownership to non-root user
RUN chown -R ctf:ctf /home/ctf

# Switch to non-root user
USER ctf

# Expose port
EXPOSE 1337

# Use socat to bind port 1337 to your challenge
CMD socat TCP-LISTEN:1337,reuseaddr,fork EXEC:"timeoute 60 /chall/run.sh"

