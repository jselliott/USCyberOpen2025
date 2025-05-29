from fastapi import FastAPI
from pydantic import BaseModel
import random
import string

app = FastAPI()

def generate_hex_string(length=16):
    # Generate a random 16-character hexadecimal string
    return ''.join(random.choices(string.hexdigits.lower(), k=length))

@app.get("/get_flag")
def get_flag():
    flag = f"SVUSCG{{{generate_hex_string()}}}"
    return {"flag": flag}

