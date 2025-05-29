# Decryptonite

A secure notes server where data is encrypted in transit. The client encrypts notes which are then decrypted by the server. The protocol includes a length-field which can cause decryption to progress past the end of the input buffer and into other values on the stack. The values on the stack are predictable and the CBC mode of operation allows for controlled decryption of the last block of ciphertext which can be used to overwrite a function pointer in the EVP_CIPHER_CTX object to start a ROP chain.

### Deployment Instructions

Run src/deploy-docker.sh to build and deploy the server. Default port is 8888, you can change the mapping in the script if you want.

### Testing Instructions

cd to the solution directory and run `./solve.py`:
```
❯ ./solve.py --help
usage: solve.py [-h] [-t TARGET] [-p PORT]

Exploit for Decryptonite.

options:
  -h, --help           show this help message and exit
  -t, --target TARGET  ip of target
  -p, --port PORT      port of target
```
