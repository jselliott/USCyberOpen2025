FROM python:3.11-slim

RUN apt-get update && apt-get install -y \
    nginx \
    supervisor \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /var/www/html

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY app.py .
COPY setup.py .
COPY flag.txt .

RUN mkdir -p templates
COPY templates/ templates/

RUN python setup.py

RUN mkdir -p /var/www/source
RUN cp app.py setup.py /var/www/source/
RUN cp -r templates /var/www/source/

RUN mkdir -p /var/www/static

COPY nginx.conf /etc/nginx/nginx.conf
COPY proxy.conf /etc/nginx/sites-available/proxy

COPY supervisord.conf /etc/supervisor/conf.d/supervisord.conf

RUN rm -f /etc/nginx/sites-enabled/default
RUN ln -s /etc/nginx/sites-available/proxy /etc/nginx/sites-enabled/
RUN mv flag.txt /flag.txt

RUN mkdir -p /var/log/nginx
RUN chown -R www-data:www-data /var/www
RUN chown -R www-data:www-data /var/log

EXPOSE 80

CMD ["/usr/bin/supervisord", "-c", "/etc/supervisor/conf.d/supervisord.conf"]
