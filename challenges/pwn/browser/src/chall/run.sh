#!/bin/bash

tmpfile=$(mktemp /tmp/XXXXXX.js)

echo "Please give me your script input to start:"

while IFS= read -r line; do
    [[ "$line" == "eof" ]] && break
    echo "$line" >> "$tmpfile"
done

./qjs $tmpfile

#rm -f "$tmpfile"

