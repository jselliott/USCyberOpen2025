FROM maven:3.9.2-eclipse-temurin-17 AS plugin-builder
WORKDIR /opt
COPY . .
RUN mvn clean package
CMD ["sleep","9999"]