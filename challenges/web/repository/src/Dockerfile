# Frontend Builder
FROM node:20 AS frontend-builder
WORKDIR /app
COPY frontend/ ./frontend/
WORKDIR /app/frontend
RUN npm install && npm run build

# Plugin builder
FROM maven:3.9.2-eclipse-temurin-17 AS plugin-builder
WORKDIR /plugins
COPY plugins .
RUN cd /plugins/LocalStorage && mvn clean package
RUN cd /plugins/HTTPStorage && mvn clean package
RUN cd /plugins/S3Storage && mvn clean package
RUN cd /plugins/AzureStorage && mvn clean package
RUN cd /plugins/GCSStorage && mvn clean package

# Backend Builder
FROM maven:3.9.2-eclipse-temurin-17 AS backend-builder
WORKDIR /app
COPY backend/ ./backend/
WORKDIR /app/backend

# Copy built frontend into backend static dir
COPY --from=frontend-builder /app/frontend/dist ./src/main/resources/static

RUN mvn clean package

# File Runtime
FROM eclipse-temurin:17-jre
WORKDIR /app

# Default local repo
RUN mkdir -p data/default
RUN echo "Your first example file!" > data/default/welcome.txt

# Copy the built Spring Boot JAR
COPY --from=backend-builder /app/backend/target/*.jar ./app.jar
COPY --from=plugin-builder /plugins/LocalStorage/target/LocalStorage-1.0.0.jar /app/plugins/storage/LocalStorage.jar
COPY --from=plugin-builder /plugins/HTTPStorage/target/HTTPStorage-1.0.0.jar /app/plugins/storage/HTTPStorage.jar
COPY --from=plugin-builder /plugins/AzureStorage/target/AzureStorage-1.0.0.jar /app/plugins/storage/AzureStorage.jar
COPY --from=plugin-builder /plugins/S3Storage/target/S3Storage-1.0.0.jar /app/plugins/storage/S3Storage.jar
COPY --from=plugin-builder /plugins/GCSStorage/target/GCSStorage-1.0.0.jar /app/plugins/storage/GCSStorage.jar

# Copy the flag into the container
COPY flag.txt /flag.txt

EXPOSE 8080
CMD ["java", "-jar", "app.jar"]
