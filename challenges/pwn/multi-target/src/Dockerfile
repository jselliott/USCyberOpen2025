FROM debian:bookworm-slim

RUN apt-get update && \
    apt-get install -y socat

# Create a non-root user for running the challenge
RUN useradd -m ctf

# Copy challenge files
COPY ./chall /chall

RUN cp /chall/flag.txt /

RUN chmod -R +x /chall/

RUN chmod 444 /chall/flag.txt
RUN chmod 444 /flag.txt

# Set ownership to non-root user
RUN chown -R ctf:ctf /home/ctf

# Switch to non-root user
#USER ctf

# Expose port
EXPOSE 1341

WORKDIR /chall

# Use socat to bind port 1337 to your challenge
CMD socat TCP-LISTEN:1341,reuseaddr,fork EXEC:"timeout 60 /chall/run.sh"

