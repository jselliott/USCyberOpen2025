# Stage 1: Build
FROM mcr.microsoft.com/dotnet/sdk:8.0 AS build
WORKDIR /src
COPY src/ ./
RUN dotnet restore
RUN dotnet publish -c Release -o /app

# Stage 2: Runtime
FROM mcr.microsoft.com/dotnet/aspnet:8.0
WORKDIR /app
ENV ASPNETCORE_URLS=http://+:80
ENV DOTNET_SYSTEM_GLOBALIZATION_INVARIANT=1

# Install ImageMagick CLI
RUN apt-get update && \
    apt-get install -y imagemagick sqlite3 libmagickwand-6.q16-6 pango1.0-tools fonts-noto-color-emoji && \
    rm -rf /var/lib/apt/lists/*


# Copy published app
COPY --from=build /app ./
COPY flag.txt /flag.txt
RUN mkdir -p /app/data /app/uploads

# Copy migration SQL for RESET.sh to find
COPY src/Data/Migrations/InitialCreate.sql /src/Data/Migrations/InitialCreate.sql

# Entrypoint: seed DB, set INVITE_CODE, then run app
COPY RESET.sh /RESET.sh
RUN chmod +x /RESET.sh
ENTRYPOINT ["/RESET.sh"]
