FROM python:3.12-slim

WORKDIR /app

RUN apt-get update && \
    apt-get install -y chromium-driver \
    libnss3 libatk-bridge2.0-0 libdrm2 libxkbcommon0 \
    libxcb1 libx11-6 libxcomposite1 libxcursor1 libxdamage1 \
    libxext6 libxfixes3 libxrandr2 libxss1 libasound2 \
    libfontconfig1 libglib2.0-0 libgtk-3-0 && \
    apt-get clean

COPY requirements.txt .
RUN pip install -r requirements.txt

COPY . .

EXPOSE 8000
CMD ["gunicorn","--timeout","120","--workers","4","-b","0.0.0.0:8000","app:create_app()"]
# CMD exec /bin/bash -c "trap : TERM INT; sleep infinity & wait"

