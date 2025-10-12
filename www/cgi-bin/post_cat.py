#!/usr/bin/env python3
import os
import sys
import requests
from urllib.parse import parse_qs

print("Content-Type: text/html\n")

# Check for POST data
method = os.environ.get('REQUEST_METHOD', 'GET')
cat_name = "Random Cat"

if method == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        params = parse_qs(post_data)
        cat_name = params.get('name', ['Random Cat'])[0]

print("<html>")
print("<head><title>Cat CGI</title></head>")
print("<body style='font-family: sans-serif; text-align: center;'>")
print(f"<h1>{cat_name}</h1>")

try:
    resp = requests.get("https://api.thecatapi.com/v1/images/search", timeout=5)
    if resp.status_code == 200:
        data = resp.json()
        if data and "url" in data[0]:
            cat_url = data[0]["url"]
            print(f"<img src='{cat_url}' alt='Random Cat'>")
except Exception as e:
    print(f"<p>Exception: {e}</p>")

print("</body></html>")