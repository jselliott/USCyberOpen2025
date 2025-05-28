# Decryptonite Writeup

> Your notes are secured with state-of-the-art end-to-end encryption. Just don't ask too many questions about what happens after they arrive.

The provided binaries implement a CLI-based notes application. The server binary implements a notes server which the client binary can connect to and send notes for temporary storage. We can add notes (up to 10 notes) and view existing notes. When adding a note, the text we put in is encrypted with a random key and initialization vector and then sent to the server, along withe initialization vector and key used to encrypt the field and the length of the original message.

```
❯ docker run --rm -it -p 8888:8888 --privileged decryptonite
Listening on port 8888
```

```
❯ docker run --rm -it -v $PWD:/w -w /w --network host decryptonite bash
root@docker-desktop:/w# ./decryptonite_client -c 127.0.0.1 -p 8888
1. List notes
2. Add a note
Choose an option (1-2): 2
Enter note to save: hello world
1. List notes
2. Add a note
Choose an option (1-2): 1
------------- Notes -------------
1. hello world
---------------------------------
```

## Decrypt out-of-bounds
The length field of the message is not checked and can be set to whatever value we want, causing decryption to progress past the end of the input buffer. The decrypted output is copied into the same buffer where the encrypted message is stored, so we have a stack buffer overflow!

## Exploit strategy
Even though we have a buffer overflow, we don't have much control over the resulting decryption of the data outside our buffer... OR DO WE?!!

As it turns out, when AES is operating in CBC mode, the decryption of the block depends on the previous ciphertext block, which we do have control over! The decryption of the block is as follows:

```
ptxt_block = prev_ctxt_block ^ F'(cur_ctxt_block, key)
```

Where `F'` is the inverse AES function. If we can predict the value of the out-of-bounds ciphertext block, we can control how it gets decrypted by altering the last controlled ciphertext block according to this formula. Using GDB we can tell that the `EVP_CIPHER_CTX` object is right after the end of the message buffer, and its first component is the pointer to the `EVP_CIPHER` used in the encryption. We can use our controlled overflow to gain control over the least significant 4 bytes of the pointer and point it to a fake cipher object!

Lucky for us, the `EVP_CIPHER` pointer that we can overwrite is used by the `EVP_CIPHER_CTX_cleanup` function, where the `cleanup` field (offset 0x28 in the `EVP_CIPHER` struct) is called in order to perform special cleanup operations for the cipher. This is normally set to NULL for `EVP_aes_128_cbc`, but we can set it to the first ROP gadget in our chain instead to get RCE.

## Exploit workflow
Putting it all together, the solution is as follows (see solve.py for details):

1. Prepare a fake `EVP_CIPHER` object at the start of the buffer where offset 0x28 is a pointer to a ROP gadget that moves the stack pointer down into the buffer
2. Place ROP gadgets for opening the flag and sending it over the existing network socket at appropriate offsets in the buffer
3. Fill up to offset 1008 in the buffer with padding
4. Encrypt the first part of the buffer using whatever IV and key (doesn't matter)
5. Calculate the value needed for the last block using the CBC decryption trick. We want to overwrite the `EVP_CIPHER` pointer with a pointer to the fake object we created at the start of our buffer. My script bruteforces this, but it should be somewhere around `0x7fffffffe6c4`.
6. Fill up the rest of the message buffer with null bytes (up to 1028 bytes)
7. Send the encrypted message in the format iv + key + length + message
8. Receive from the socket to get the flag!
