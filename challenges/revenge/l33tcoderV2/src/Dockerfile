FROM python:3-alpine

WORKDIR /app

COPY app .
COPY flag.txt /flag.txt

RUN pip install flask gunicorn uscg-leetcode-validator
RUN python3 generate-tests.py

ENV PIP_INDEX_URL=127.0.0.1:5000/simple

CMD ["gunicorn","-w","4","-b","0.0.0.0:5000","app:app"]