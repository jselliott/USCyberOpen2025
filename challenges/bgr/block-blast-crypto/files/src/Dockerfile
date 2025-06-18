FROM python:3.11-alpine

WORKDIR /app
COPY challenge.py .
COPY flag.txt .

RUN apk add socat
RUN pip install --no-cache-dir pycryptodome

EXPOSE 1337

CMD ["socat", "TCP-LISTEN:1337,reuseaddr,fork", "EXEC:'python3 challenge.py',stderr"]
