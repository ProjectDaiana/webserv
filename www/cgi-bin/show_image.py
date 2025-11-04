#!/usr/bin/env python3
"""
CGI that saves uploaded images and displays them
Works with Transfer-Encoding: chunked
"""

import sys
import os
import base64
import time

# Get script directory and build absolute paths
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
UPLOAD_DIR = os.path.join(SCRIPT_DIR, '..', 'html', 'uploads')
SAVED_IMAGE = os.path.join(UPLOAD_DIR, 'latest_upload.jpg')
METADATA_FILE = os.path.join(UPLOAD_DIR, 'latest_metadata.txt')

# Read POST data from stdin
content_length = os.environ.get('CONTENT_LENGTH', '')
content_type = os.environ.get('CONTENT_TYPE', 'application/octet-stream')
method = os.environ.get('REQUEST_METHOD', 'GET')

image_data = b''
was_chunked = False

if method == 'POST':
    # Read binary data
    image_data = sys.stdin.buffer.read()
    was_chunked = (not content_length or content_length == '')
    
    # Save the uploaded image
    if len(image_data) > 0:
        try:
            os.makedirs(UPLOAD_DIR, exist_ok=True)
            with open(SAVED_IMAGE, 'wb') as f:
                f.write(image_data)
            
            # Save metadata
            with open(METADATA_FILE, 'w') as f:
                f.write(f"Upload Time: {time.ctime()}\n")
                f.write(f"Size: {len(image_data)} bytes\n")
                f.write(f"Encoding: {'Transfer-Encoding: chunked' if was_chunked else 'Content-Length'}\n")
                f.write(f"Content-Type: {content_type}\n")
        except Exception as e:
            pass
else:
    # GET request - load previously saved image
    if os.path.exists(SAVED_IMAGE):
        with open(SAVED_IMAGE, 'rb') as f:
            image_data = f.read()
        
        # Load metadata
        metadata = {}
        if os.path.exists(METADATA_FILE):
            with open(METADATA_FILE, 'r') as f:
                for line in f:
                    if ':' in line:
                        key, value = line.strip().split(':', 1)
                        metadata[key.strip()] = value.strip()
        
        was_chunked = 'chunked' in metadata.get('Encoding', '').lower()
        content_type = metadata.get('Content-Type', 'image/jpeg')

# Output HTTP headers
# # print("Content-Type: text/html")
# print()

# Generate HTML
html = f"""<!DOCTYPE html>
<html>
<head>
    <title>Chunked Upload Test</title>
    <style>
        body {{ font-family: Arial; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; display: flex; align-items: center; justify-content: center; margin: 0; padding: 20px; }}
        .container {{ background: white; padding: 40px; border-radius: 20px; box-shadow: 0 20px 60px rgba(0,0,0,0.3); max-width: 800px; }}
        h1 {{ color: #2ecc71; text-align: center; }}
        .status {{ font-size: 4em; text-align: center; margin: 20px 0; }}
        .info {{ background: #f8f9fa; padding: 20px; border-radius: 10px; margin: 20px 0; }}
        .image-container {{ text-align: center; margin: 30px 0; }}
        .image-container img {{ max-width: 100%; border: 4px solid #ddd; border-radius: 8px; }}
    </style>
</head>
<body>
    <div class="container">
        <h1>{'Image Uploaded!' if method == 'POST' else 'Latest Image' if len(image_data) > 0 else 'No Image Yet'}</h1>
        {f'<div class="info"><p><strong>Encoding:</strong> {"Transfer-Encoding: chunked" if was_chunked else "Content-Length"}</p><p><strong>Size:</strong> {len(image_data)} bytes</p></div>' if len(image_data) > 0 else '<p style="text-align:center;">Run: cat chunked_request.bin | nc localhost 8080</p>'}
        {f'<div class="image-container"><img src="data:{content_type};base64,{base64.b64encode(image_data).decode()}" alt="Uploaded"></div>' if len(image_data) > 0 else ''}
    </div>
</body>
</html>"""

print(html)
