#!/usr/bin/env python3
import cgi
import os

UPLOAD_DIR = "uploads"  # relative to cgi-bin


form = cgi.FieldStorage()
if "file" in form:
    fileitem = form["file"]
    if fileitem.filename:
        filename = os.path.basename(fileitem.filename)
        filepath = os.path.join(os.path.dirname(__file__), UPLOAD_DIR, filename)
        with open(filepath, "wb") as f:
            f.write(fileitem.file.read())
        print(f"<p>File '{filename}' uploaded successfully!</p>")
    else:
        print("<p>No file was uploaded.</p>")
else:
    print("<p>No file field in form.</p>")