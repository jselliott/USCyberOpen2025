ELF_bytes = #"REDACTED"
key = #REDACTED
def not_so_fast(ELF_bytes,key):
    message = ""
    for i in range(len(ELF_bytes)):
        message += chr(ord(ELF_bytes[i]) + key)
    return message

with open("bytes.txt","a") as file:
    file.write("You want your ELF challenge so bad? Well here it is!\n")
    file.write(not_so_fast(ELF_bytes,key))