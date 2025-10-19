#!/usr/bin/env python3
import os
import sys
import cgi
import base64

# minimal headers
print("Content-Type: text/html")
print()

# Diagnostics: expose env values (hidden by default)
method = os.environ.get('REQUEST_METHOD', 'GET')
cl = os.environ.get('CONTENT_LENGTH', '')
ct = os.environ.get('CONTENT_TYPE', '')
print("<pre style='display:none'>")
print(f"REQUEST_METHOD: {method}")
print(f"CONTENT_LENGTH: {cl}")
print(f"CONTENT_TYPE: {ct}")
print("</pre>")

# Parse form
# fieldstorage reads from sys.stdin by default using CONTENT_LENGTH/CONTENT_TYPE
form = cgi.FieldStorage()

# Visible debug: parsed keys
keys = list(form.keys()) if hasattr(form, 'keys') else []
print("<div style='font-family:monospace; font-size:12px; margin:8px;'>")
print("<strong>Parsed form keys:</strong> " + ", ".join(keys))
print("</div>")

print("""
<html>
<head>
    <title>Cat Photo Upload</title>
    <style>
        body { font-family: sans-serif; text-align: center; padding: 50px; }
        .result { background: #e8f5e9; padding: 20px; border-radius: 8px; margin: 20px auto; max-width: 500px; }
        input, button { padding: 10px; font-size: 16px; margin: 5px; }
        button { background: #4CAF50; color: white; border: none; border-radius: 4px; cursor: pointer; }
    </style>
</head>
<body>
""")

if method == 'POST':
    fileitem = form['photo'] if 'photo' in form else None
    cat_name = form.getvalue('name', '')
    filename = getattr(fileitem, 'filename', None)
    if fileitem is not None and filename:
        # read uploaded bytes
        data = fileitem.file.read()
        # show size and image inline
        print(f"<div class='result'>")
        print(f"<h1>🐱 Hello, {cat_name}!</h1>")
        print(f"<p>Photo '{fileitem.filename}' uploaded successfully!</p>")
        mime = getattr(fileitem, 'type', 'application/octet-stream') or 'application/octet-stream'
        b64 = base64.b64encode(data).decode('ascii')
        print(f"<img src='data:{mime};base64,{b64}' style='max-width:300px;'>")
        print("</div>")
    else:
        print("<div class='result'><p>No photo uploaded or 'photo' field missing.</p></div>")
else:
    print("<h1>Welcome! 🐱</h1>")
    print("<p>Upload a photo of your cat below:</p>")

print("""
<div style='margin: 30px;'>
<form method='POST' action='/cgi-bin/post_foto.py' enctype='multipart/form-data'>
  <input type='text' name='name' placeholder='Cat name' required><br>
  <input type='file' name='photo' accept='image/*' required><br>
  <button type='submit'>Upload Cat Photo</button>
</form>
</div>
""")

print(f"<p><small>Request Method: {method}</small></p>")
print("</body></html>")