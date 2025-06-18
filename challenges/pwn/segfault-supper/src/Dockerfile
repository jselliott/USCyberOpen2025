FROM ubuntu:22.04

# setup xinetd
RUN apt-get -qq update && \
    apt-get -qq install -y --no-install-recommends xinetd sqlite3
RUN useradd --system chal
COPY base-container/* /

# setup chal dir
WORKDIR /chal
# Copy flag
COPY flag.txt ./
# Copy server binary
COPY segfault_supper ./
# Copy db schema
COPY schema.sql ./

# ensure chal dir is read only
RUN chown -R root:root ./* && \
    chmod -R o-w ./*

# Create writable directory for database
RUN mkdir -m 777 db

# Generate database
RUN sqlite3 db/food.db < schema.sql
# grant write permissions to the database
RUN chmod a+w db/food.db

EXPOSE 1337

RUN chmod +x /run_chal.sh
CMD ["/usr/sbin/xinetd", "-syslog", "local0", "-dontfork", "-f", "/xinetd.conf"]
