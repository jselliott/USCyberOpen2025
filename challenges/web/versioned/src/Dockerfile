# Stage 1: Build frontend
FROM node:20-alpine AS frontend-build
WORKDIR /app
COPY frontend/ .
RUN npm install && npm run build

# Stage 2: Build backend
FROM golang:1.22-alpine AS backend-build
WORKDIR /app
COPY backend/ .
RUN go build -o server .

# Stage 3: Final image with nginx and app
FROM nginx:alpine
WORKDIR /app

# Copy built frontend
COPY --from=frontend-build /app/dist /usr/share/nginx/html

# Copy nginx config
COPY config/nginx.conf /etc/nginx/conf.d/default.conf

# Copy backend binary
COPY --from=backend-build /app/server /usr/bin/server

# Expose ports
EXPOSE 80

# Start both backend and nginx
CMD ["/bin/sh", "-c", "/usr/bin/server & nginx -g 'daemon off;'"]
