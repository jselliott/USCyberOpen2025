FROM tiangolo/uvicorn-gunicorn-fastapi:python3.11

COPY ./requirements.txt /app/requirements.txt

RUN pip install --no-cache-dir --upgrade -r /app/requirements.txt

RUN mkdir -p /app/users/admin && \
    echo "mysupersecretadminpassword_asdfhwuirhfqcnapuhfpwq" > /app/users/admin/pass.txt && \
    ln -s /flag.txt /app/users/admin/flag.txt

COPY ./app /app

EXPOSE 80