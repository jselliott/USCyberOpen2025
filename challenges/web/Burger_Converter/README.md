# Metric-to-‘Imperial’ Conversion Portal CTF Challenge

## Overview
This is a single-container, Flask-powered CTF challenge. The goal is to exploit a CORS flaw to change the admin’s password, then log in and read the flag.

In order for this challenge to work properly, the website must be deployed via HTTPS. Chrome will not allow the session cookie to be used via CORS without being sent across a secure channel.

## Running the Challenge

1. **Build the Docker image:**
   ```bash
   docker build -t patriotic-convertor .
   ```
2. **Run the container:**
   ```bash
   docker run -p 8000:8000 patriotic-convertor
   ```
3. **Access the app:**
   Open [http://localhost:8000](http://localhost:8000) in your browser.

## API Endpoints
See the challenge description for the full API surface. Key endpoints:
- `/api/signup`, `/api/login`, `/api/logout`, `/api/change-password` (CORS vuln)
- `/api/conversions`, `/api/conversions/<id>`
- `/api/profile`, `/profile` (HTML, shows flag if admin)

## CORS Flaw
The `/api/change-password` endpoint reflects the `Origin` header and allows credentials. All other endpoints are locked to `https://patriotic-convertor.ctf`.

## Admin Bot
A Selenium-based admin bot runs every 60s, logs in as admin, and visits unreviewed player-supplied URLs.

## Hints

**Hint 1:**
```bash
curl -X OPTIONS -H "Origin:https://evil.eagle" \
     -H "Access-Control-Request-Method:PUT" \
     -I http://patriotic-convertor.ctf/api/change-password
```

**Hint 2:**
Host the provided `attack.html` to exploit the CORS flaw.

## Win Condition
Change the admin password, log in as admin, and visit `/profile` to see the flag: `CTF{Stars-Bars-And-Headers}`.

---

