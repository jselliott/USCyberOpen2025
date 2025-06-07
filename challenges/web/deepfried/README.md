# Deep-Fried-inator 🤘

Welcome to the Deep-Fried-inator CTF challenge! Crisp your memes, hack the stack, and snatch the flag.

## Build & Run

```
docker build -t deep-fried-inator .
docker run -p 8080:80 --rm deep-fried-inator
```

- App: http://localhost:8080
- On start, an invite code is generated and exported as `INVITE_CODE` env var.

## Challenge Flow

1. **Register**: Leak the invite code by providing a special character to trigger a stack trace.
2. **Upload**: Submit an image (≤2MB, image/* MIME).
3. **Deep-Fry**: App shells out to ImageMagick, trashes colors, overlays a random emoji.
4. **Vulnerability**: The app re-reads the processed file using a path derived from your original filename, with no sanitization on the emojis. Only one emoji can be placed into the emoji field, but multiple emoji fields can be specified in the POST. Pay attention to how this operates when using it as intended with multiple emoji selections.

Path traversal lets you read `/flag.txt` as a meme which is saved to the SQLite database. A successful directory traversal will result in a filename such as `Profile_DEEPFRIED/../../../../flag.txt`, which is then read without proper sanitization.
5. **Flag**: The flag will be committed to the database as a blob, then is loaded as an image when browsing to it.

## Security Posture

- Rate-limited, MIME-checked, and size-capped.
- **Deliberate path traversal** in file read after ImageMagick. While we're calling a shell command, this should not be command injection.
- `/flag.txt` is not web-served.
- Invite code is only leaked if run with `ASPNETCORE_ENVIRONMENT=Development` and `ASPNETCORE_DETAILEDERRORS=true` (error page dumps env vars).

## RESET

To reset DB and invite code, restart the container.

## Author Write-up

This challenge demonstrates a classic read-side path traversal, where user input is used to construct a file path for reading, not just writing. The “deep-fry” meme logic is a fun wrapper for the vulnerability. The flag is only accessible by exploiting the traversal, not by direct HTTP access. The invite code mechanism and error page leak add a secondary challenge for players.

---

**UI and error messages are styled in peak Gen-Z/techno jargon for max memeage.**

Good luck, cybernaut! 🚀
