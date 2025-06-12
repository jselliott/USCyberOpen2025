FROM python:3-alpine

WORKDIR /app

COPY app .
COPY flag.txt /flag.txt

RUN pip install flask gunicorn uscg-leetcode-validator-2
RUN python3 generate-tests.py

CMD ["gunicorn","-w","4","-b","0.0.0.0:5000","app:app"]