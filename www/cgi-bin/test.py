#!/usr/bin/env python3

import requests
import time 

print("<html>")
print("<head><title>Cat CGI</title></head>")
print("<body style='font-family: sans-serif; text-align: center;'>")
print("<h1>Here's a random cat for you :)")

# Use thecatapi (free random cat images)
try:
    resp = requests.get("https://api.thecatapi.com/v1/images/search")
    if resp.status_code == 200:
        data = resp.json()
        if data and "url" in data[0]:
            cat_url = data[0]["url"]
            print(f"<img src='{cat_url}' alt='Random Cat' style='max-width:600px; border-radius:12px; box-shadow:0 0 10px #888;'>")
        else:
            print("<p>Couldn’t fetch a cat</p>")
    else:
        print("<p>Error fetching cat API</p>")
except Exception as e:
    print(f"<p>Exception: {e}</p>")
# while True:
#     pass
print("</body>")
print("</html>")

