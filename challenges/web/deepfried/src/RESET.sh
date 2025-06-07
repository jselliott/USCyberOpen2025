#!/bin/bash
set -e
# Generate random invite code
INVITE_CODE=$(head -c 64 /dev/urandom | base64 | tr -dc 'A-Z0-9' | head -c 64)
export INVITE_CODE

# Seed DB if not exists
if [ ! -f /app/data/deepfried.db ]; then
  sqlite3 /app/data/deepfried.db < /src/Data/Migrations/InitialCreate.sql
  sqlite3 /app/data/deepfried.db "INSERT INTO InviteCodes (Code) VALUES ('$INVITE_CODE');"
fi

# Start the app
dotnet DeepFriedinator.dll
