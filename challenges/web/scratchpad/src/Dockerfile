FROM node:20-alpine AS frontend-build
WORKDIR /app
COPY frontend/ .
RUN npm install && npm run build

FROM golang:1.24-alpine AS backend-build
WORKDIR /app
COPY backend/ .
RUN apk add alpine-sdk
RUN CGO_ENABLED=1 go build -o server .

FROM alpine:latest
WORKDIR /app
RUN apk add --no-cache nginx supervisor git sqlite sqlite-libs uuidgen
RUN git config --global user.email "scratchpad@uscg.local" && git config --global user.name "Scratchpad App" && git config --global init.defaultBranch master
COPY --from=frontend-build /app/dist ./html
COPY --from=backend-build /app/server ./server
COPY config/nginx.conf /etc/nginx/nginx.conf
COPY config/supervisord.conf /etc/supervisord.conf
COPY flag.txt /flag.txt
COPY entrypoint.sh .
RUN chmod +x entrypoint.sh

EXPOSE 80

CMD ["./entrypoint.sh"]
