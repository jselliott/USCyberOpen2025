# From https://github.com/tiangolo/uvicorn-gunicorn-fastapi-docker.git

FROM python:3.11-slim

# Install FastAPI and Uvicorn
RUN pip install --no-cache-dir fastapi uvicorn

# Copy application code
COPY ./app /app
WORKDIR /app

# Expose port 8002
EXPOSE 8002

# Run the FastAPI app with Uvicorn
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8002"]