FROM python:3.13.3-alpine


RUN apk add --no-cache sqlite
RUN pip install Flask gunicorn

WORKDIR /app

COPY ./src .
COPY flag.txt /flag.txt

EXPOSE 8080

CMD ["gunicorn", "-b", "0.0.0.0:8080", "main:app"]
