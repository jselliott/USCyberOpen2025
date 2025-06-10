FROM python:3.10-alpine

WORKDIR /app
RUN apk add alpine-sdk gcc musl-dev linux-headers
RUN pip install randcrack pwntools tqdm

COPY exploit.py .

ENTRYPOINT ["python3", "exploit.py"]