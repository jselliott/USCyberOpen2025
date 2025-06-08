# Scratchpad

## Solution

There are two different injection vulnerabilites that must be used together in order to get the flag. The player needs to find a way to access the flag file that is in the admin user's folder in a past revision that has been blanked out. In order to do this, they actually need to be able to find the GUID name of the folder.

There is an SQL injection vulnerability in the username check function that can be used to very quickly enumerate the folder name. Next, they need to be able to somehow load this file even though it is in the other folder outside of their own git repo. To do this they can take advantage of the second vulnerability which is in the revision loader. The player is able to inject another path with the -C flag to change to another directory, and even though they don't know the actual commit hash, they can find it relative to HEAD. By sending this payload to the revision handler with the admin folder, the flag is revealed:

```
{"action":"-C /app/data/e3cb0962-7367-42cf-b279-dee6d80ab770 show","hash":"HEAD~1"}
```

This challenge requires not only some SQL and command injection skill but also some research into the workings of Git to be able to properly implement the exploit.