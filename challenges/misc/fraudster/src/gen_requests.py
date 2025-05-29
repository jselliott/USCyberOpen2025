import requests
import subprocess
import random
import socket
import time
from collections import OrderedDict

# For SOCKS5 proxy support
import sys
try:
    import socks
    import requests
except ImportError:
    pass  # Will error if not installed, but only needed for proxy


# Define the URL of the API
url = "http://localhost:8002/get_flag"

headers = {
    'User-Agent': 'checker',
    'X-Flag-Checker': 'true',
    'Accept': 'application/json'
}

# Function to make a single request
def make_request(header, verbose=False):
    response = requests.get(url, headers=header)
    if verbose:
        print(response.json())

# Function to make a request over SOCKS5 proxy. Point this to Singapore
def make_socks5_request(header, verbose=False):
    """
    Makes a request to the same URL using a SOCKS5 proxy on localhost:9050 (default Tor port).
    Requires 'requests[socks]' to be installed.
    """
    proxies = {
        'http': 'socks5h://localhost:9050',
        'https': 'socks5h://localhost:9050',
    }
    response = requests.get(url, headers=header, proxies=proxies, timeout=10)
    if verbose:
        print(response.json())

# Generate a random point to send across the socks5 proxy
fraud_point = random.randint(200, 800)

print(f"Fraudster request will be {fraud_point}")

time.sleep(1)

# Make 1000 requests
for i in range(1000):
    # On the random point, use SOCKS5 proxy for RTT spike
    if i == fraud_point:
        print("Executing request over SOCKS5 proxy for RTT spike")
        try:
            make_socks5_request(headers, verbose=True)
        except Exception as e:
            print(f"SOCKS5 proxy request failed: {e}")
        continue

    # Make the request
    make_request(headers)
