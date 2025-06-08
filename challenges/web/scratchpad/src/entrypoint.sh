#!/bin/sh
set -e

DB_PATH="/app/ctf.db"
USERS_DIR="/app/data"
FLAG=$(cat /flag.txt)
FOLDER=$(uuidgen)
FOLDER_PATH="$USERS_DIR/$FOLDER"

echo $FOLDER

mkdir -p "$FOLDER_PATH"

cd "$FOLDER_PATH"
git init
echo "$FLAG" > scratchpad.txt
git add scratchpad.txt
git commit -m "Add flag"
echo "" > scratchpad.txt
git add scratchpad.txt
git commit -m "Blank file"
cd -

sqlite3 "$DB_PATH" <<EOF
CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password TEXT, folder TEXT, filename TEXT);
INSERT INTO users (username, password, folder, filename) VALUES ('admin', 'temppassword', '$FOLDER','scratchpad.txt');
EOF

rm /flag.txt

/usr/bin/supervisord -c /etc/supervisord.conf
