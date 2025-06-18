FROM python:3.10-slim

WORKDIR /app

# Install dependencies
COPY requirements.txt /app/
RUN pip install --no-cache-dir -r requirements.txt

# Copy application code
COPY app /app

# Set default Flask environment variables
ENV FLASK_APP=app.py
ENV FLASK_ENV=production

EXPOSE 5000

RUN pip install gunicorn

# Run the app
CMD ["gunicorn", "-b", "0.0.0.0:8000", "app:app"]