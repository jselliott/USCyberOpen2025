from fastapi import FastAPI, HTTPException, status, Depends
from pydantic import BaseModel
from pathlib import Path
import base64 # Added for token encoding
import secrets # Added for nonce generation
from datetime import datetime, timezone # Added for timestamp
import hashlib
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
from typing import List # Added for List response model
import os # For file and directory operations
from key import key

app = FastAPI()

# Base directory for all user-specific data
USERS_BASE_DIR = Path("users")

# --- Pydantic Models ---
class UserCredentials(BaseModel):
    username: bytes
    password: bytes

class NoteBase(BaseModel):
    title: bytes

class NoteCreate(NoteBase):
    text: bytes

def sanitize_username(username):
    """Removes any characters that are not lowercase English letters."""
    return bytes(filter(lambda x: 96 < x < 123, username))

def hmac(token):
    return hashlib.sha256(key.encode() + token).digest()

# --- Authentication Dependency ---
reusable_bearer = HTTPBearer()

# Length of the HMAC output in hex (SHA256 produces 32 bytes -> 64 hex chars)
HMAC_HEX_LENGTH = hashlib.sha256().digest_size * 2
TOKEN_VALIDITY_SECONDS = 5 * 60  # 5 minutes

def parse_token_data(token):
    data = {}
    for kv in token.split(b'&'):
        if b'=' in kv:
            key, value = kv.split(b'=')
            data[key] = value
    return data

async def get_current_username(credentials: HTTPAuthorizationCredentials = Depends(reusable_bearer)):
    """
    Dependency to get the current username from a Bearer token.
    Decodes the token, extracts the username, and performs basic validation.
    The token is expected to be Base64 encoded. Its decoded content should be
    a query string like "len=...&nonce=...&ts=...&user=<username>"
    followed by its HMAC in hex.
    """
    token = credentials.credentials
    try:
        decoded_token_bytes = base64.b64decode(token)
        decoded_token_str = decoded_token_bytes
    except (base64.binascii.Error, UnicodeDecodeError) as e:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail=f"Invalid token encoding or format: {str(e)}",
            headers={"WWW-Authenticate": "Bearer"},
        )

    if len(decoded_token_str) < HMAC_HEX_LENGTH:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid token: too short to contain HMAC.",
            headers={"WWW-Authenticate": "Bearer"},
        )

    token_payload_str = decoded_token_str[:-HMAC_HEX_LENGTH]
    received_hmac_hex = decoded_token_str[-HMAC_HEX_LENGTH:]

    # Verify HMAC
    expected_hmac_bytes = hmac(token_payload_str)
    expected_hmac_hex = expected_hmac_bytes.hex()

    if not secrets.compare_digest(received_hmac_hex, expected_hmac_hex.encode()):
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid token: HMAC verification failed.",
            headers={"WWW-Authenticate": "Bearer"},
        )

    token_data = parse_token_data(token_payload_str)
    print(token_data)
    raw_username_from_token = token_data.get(b"user")
    token_ts_str = token_data.get(b"ts")

    if not raw_username_from_token:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid token: user field missing from token payload.",
            headers={"WWW-Authenticate": "Bearer"},
        )
    
    if not token_ts_str:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid token: timestamp missing.",
            headers={"WWW-Authenticate": "Bearer"},
        )

    try:
        token_ts = int(token_ts_str.decode())
    except ValueError:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid token: timestamp format is invalid.",
            headers={"WWW-Authenticate": "Bearer"},
        )

    current_ts = int(datetime.now(timezone.utc).timestamp())
    if current_ts - token_ts > TOKEN_VALIDITY_SECONDS:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Token has expired.",
            headers={"WWW-Authenticate": "Bearer"},
        )

    # Sanitize the username extracted from the token
    username = sanitize_username(raw_username_from_token)

    # Check if the username is empty after sanitization
    if not username:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid token: username field results in an empty string after sanitization.",
            headers={"WWW-Authenticate": "Bearer"},
        )

    user_dir = USERS_BASE_DIR / username.decode()  # Path segments must be str
    if not user_dir.is_dir():
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid token: user does not exist.",
            headers={"WWW-Authenticate": "Bearer"},
        )
    return username

@app.post("/register")
async def register_user(credentials: UserCredentials):
    """
    Registers a new user by creating a directory for them
    and storing their password in a file.

    - **username**: The username for the new user.
    - **password**: The password for the new user.

    **Important Security Warning:** This implementation stores passwords in plaintext,
    which is highly insecure and should NEVER be done in a production environment.
    Always hash passwords securely (e.g., using bcrypt or Argon2).
    """
    username = credentials.username
    sanitized_username = sanitize_username(username)
    password = credentials.password

    # Validate sanitized username: must not be empty after sanitization
    if not sanitized_username:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Invalid username. After sanitization, the username is empty.",
        )

    try:
        # Ensure the base 'users' directory exists
        USERS_BASE_DIR.mkdir(exist_ok=True)
        
        user_dir = USERS_BASE_DIR / sanitized_username.decode() # Path segments must be str
        
        if user_dir.exists():
            raise HTTPException(
                status_code=status.HTTP_409_CONFLICT,
                detail=f"User '{sanitized_username.decode()}' already exists.",
            )
        
        user_dir.mkdir()

        # Write the password to pass.txt inside the user's directory
        password_file_path = user_dir / "pass.txt"
        with open(password_file_path, "w") as f:
            f.write(password.decode()) # Write password as string
            
        return {"message": f"User '{sanitized_username.decode()}' registered successfully."}

    except OSError as e:
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to register user '{sanitized_username.decode()}': {str(e)}",
        )

@app.post("/login")
async def login_user(credentials: UserCredentials):
    """
    Logs in a user by verifying their password.
    On success, returns a 200 OK with a JSON body containing the
    Base64 encoded authentication token.
    The token format before encoding is: "nonce=<random_16_hex>&ts=<timestamp>&user=<username>"

    - **username**: The username of the user.
    - **password**: The password of the user.

    """
    username = credentials.username
    sanitized_username = sanitize_username(username)
    password = credentials.password

    # Validate sanitized username: must not be empty after sanitization
    if not sanitized_username:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Invalid username. After sanitization, the username is empty.",
        )

    user_dir = USERS_BASE_DIR / sanitized_username.decode() # Path segments must be str
    password_file_path = user_dir / "pass.txt"

    if not user_dir.is_dir() or not password_file_path.is_file():
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid username or password.",
            headers={"WWW-Authenticate": "Bearer"}, # Standard for 401
        )

    try:
        with open(password_file_path, "r") as f:
            stored_password = f.read().strip().encode('utf-8') # Read as string, then encode to bytes
    except OSError:
        # This implies an issue reading a file that should exist
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Could not process user credentials.",
        )

    if stored_password != password:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid username or password.",
            headers={"WWW-Authenticate": "Bearer"},
        )

    # Generate token components
    nonce_bytes = secrets.token_hex(8).encode('utf-8')
    ts_bytes = str(int(datetime.now(timezone.utc).timestamp())).encode('utf-8')

    # Construct the main part of the token
    payload_part = b"nonce=" + nonce_bytes + b"&ts=" + ts_bytes + b"&user=" + sanitized_username
    token_data_part = b"len=" + str(len(payload_part)).encode('utf-8') + b"&" + payload_part

    # Calculate HMAC for the token data part
    calculated_hmac_hex_str = hmac(token_data_part).hex() # hmac returns bytes, .hex() returns str
    token_string_with_hmac = token_data_part + calculated_hmac_hex_str.encode('utf-8') # append hmac_hex as bytes

    # Base64 encode the token string
    # token_string_with_hmac is already bytes
    base64_encoded_token = base64.b64encode(token_string_with_hmac).decode('utf-8')
    # Return the token directly in the response body
    return {"token": base64_encoded_token}

# --- Notes Endpoints ---
@app.post("/notes", response_model=NoteBase, status_code=status.HTTP_201_CREATED)
async def write_user_note(
    note_payload: NoteCreate,
    current_username: bytes = Depends(get_current_username)
):
    """
    Writes a note for the authenticated user.
    The note title must be all lowercase letters, not empty, and cannot be 'pass'.
    Requires authentication via Bearer token.
    """
    title = note_payload.title
    note_text = note_payload.text

    # Validate title: must be all lowercase ASCII letters and not empty
    if not title or not all(97 <= b <= 122 for b in title): # 97 is 'a', 122 is 'z'
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Invalid title. Title must consist only of lowercase letters and cannot be empty.",
        )
    if title == b"pass":
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Invalid title. Title cannot be 'pass'.",
        )

    user_note_dir = USERS_BASE_DIR / current_username.decode() # Path segments must be str
    note_file_path = user_note_dir / (title.decode() + ".txt") # Filename parts must be str

    try:
        with open(note_file_path, "wb") as f: # Write in binary mode
            f.write(note_text)
        # Return just the title as confirmation
        return NoteBase(title=title)
    except OSError as e:
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to write note '{title.decode()}': {str(e)}",
        )

@app.get("/notes/{note_title}", response_model=NoteCreate)
async def read_user_note(
    note_title: str,
    current_username: bytes = Depends(get_current_username)
):
    """
    Reads a specific note for the authenticated user.
    Requires authentication via Bearer token.
    """
    note_title_bytes = note_title.encode('utf-8') # Convert path param to bytes
    if not note_title_bytes or not all(97 <= b <= 122 for b in note_title_bytes):
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Invalid note title format. Title must consist only of lowercase letters and cannot be empty.",
        )
    if note_title == "pass": # Prevent reading the password file via this endpoint
        raise HTTPException(
            status_code=status.HTTP_403_FORBIDDEN,
            detail="Access to this resource name is forbidden.",
        )

    note_file_path = USERS_BASE_DIR / current_username.decode() / (note_title + ".txt")

    if not note_file_path.is_file():
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Note '{note_title}' not found.",
        )

    try:
        note_data = note_file_path.read_bytes() # Read as bytes
        return NoteCreate(title=note_title_bytes, text=note_data)
    except OSError as e:
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to read note '{note_title}': {str(e)}", # note_title is still str here for msg
        )

@app.get("/notes", response_model=List[NoteBase])
async def list_user_notes(current_username: bytes = Depends(get_current_username)):
    """
    Lists all notes for the authenticated user.
    Excludes the 'pass.txt' file.
    Requires authentication via Bearer token.
    """
    user_notes_dir = USERS_BASE_DIR / current_username.decode() # Path segments must be str
    notes_list = []

    if not user_notes_dir.is_dir():
        # This case should ideally be caught by get_current_username,
        # but as a safeguard:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="User directory not found. Cannot list notes."
        )

    try:
        for item in user_notes_dir.iterdir():
            if item.is_file() and item.name.endswith(".txt"): # item.name is str
                note_title_str = item.stem # .stem gives the filename without the last suffix (str)
                if note_title_str != "pass":
                    notes_list.append(NoteBase(title=note_title_str.encode('utf-8')))
        return notes_list
    except OSError as e:
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to list notes: {str(e)}",
        )
