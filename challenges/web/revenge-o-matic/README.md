# Beg-o-Matic 3000

A next.js-based app where the players must beg the admin to give them the flag. It highlights a real problem with Next.js where devs may assume that randomized "secure" server action IDs can't be known by an attacker, and so further authentication of requests isn't needed. Players must use blind CSS exfiltration to bypass a restrictive CSP and steal the ID needed to approve their own request.

### Deployment Instructions

Run src/deploy-docker.sh to build and deploy the app
