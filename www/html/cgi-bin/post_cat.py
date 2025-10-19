#!/usr/bin/env python3
import os
import sys
from urllib.parse import parse_qs

print("Content-Type: text/html")
print()

method = os.environ.get('REQUEST_METHOD', 'GET')
cat_name = None
print("""
<html>
<head>
    <title>Cat Name Form</title>
    <style>
        body {
            font-family: sans-serif;
            text-align: center;
            padding: 50px;
        }
        .result {
            background: #e8f5e9;
            padding: 20px;
            border-radius: 8px;
            margin: 20px auto;
            max-width: 500px;
        }
        input, button {
            padding: 10px;
            font-size: 16px;
            margin: 5px;
        }
        button {
            background: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }
    </style>
</head>
<body>
""")

# Handle POST (form submission)
if method == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        params = parse_qs(post_data)
        cat_name = params.get('name', [''])[0]
        
        print("<div class='result'>")
        print(f"<h1>🐱 Hello, {cat_name}!</h1>")
        print(f"<p>Your cat's name has been received via POST!</p>")
        print(f"<p><small>POST data: {post_data}</small></p>")
        print("</div>")
else:
    # GET - first visit
    print("<h1>Welcome! 🐱</h1>")
    print("<p>Enter your cat's name below:</p>")

# Always show the form
print("<div style='margin: 30px;'>")
print("<form method='POST' action='/cgi-bin/post_cat.py'>")
print("  <input type='text' name='name' placeholder='Cat name' required>")
print("  <button type='submit'>Submit Cat Name</button>")
print("</form>")
print("</div>")

# Debug info
print(f"<p><small>Request Method: {method}</small></p>")

print("</body></html>")