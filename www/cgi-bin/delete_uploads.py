#!/usr/bin/env python3
import os
import cgi
import sys

UPLOAD_DIR = "uploads"

# Get request method and uploads directory path
request_method = os.environ.get("REQUEST_METHOD", "GET")
script_dir = os.path.dirname(__file__)
uploads_path = os.path.join(script_dir, UPLOAD_DIR)

# Handle POST - return only filename for server to delete
# if request_method == "POST":
#     form = cgi.FieldStorage()
#     if "delete" in form:
#         filename = form.getvalue("delete")
#         # Return just the filename so server can delete it
#         sys.stdout.write(filename)
#     else:
#         sys.stdout.write("Error: No file specified")
#     sys.exit(0)

# Handle DELETE - return only filename for server to delete
if request_method == "DELETE":
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    if content_length > 0:
        filename = sys.stdin.read(content_length).strip()
        # Return just the filename so server can delete it
        sys.stdout.write(filename)
    else:
        sys.stdout.write("Error: No filename provided")
    sys.exit(0)

# Handle GET - display file list with delete buttons
print("Content-Type: text/html\n")
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("<title>Manage Uploaded Files</title>")
print("<style>")
print("  body { font-family: Arial, sans-serif; max-width: 900px; margin: 50px auto; padding: 20px; background: #f9f9f9; }")
print("  h1 { color: #333; text-align: center; }")
print("  .file-list { list-style: none; padding: 0; }")
print("  .file-item { display: flex; justify-content: space-between; align-items: center;")
print("               padding: 15px; margin: 10px 0; background: white; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }")
print("  .file-name { flex-grow: 1; font-weight: 500; }")
print("  .file-size { color: #666; margin-right: 15px; }")
print("  .delete-btn { background: #dc3545; color: white; border: none;")
print("                padding: 8px 20px; border-radius: 5px; cursor: pointer; font-weight: bold; }")
print("  .delete-btn:hover { background: #c82333; }")
print("  .no-files { color: #666; font-style: italic; text-align: center; padding: 40px; }")
print("  .message { text-align: center; padding: 10px; margin: 10px 0; border-radius: 5px; }")
print("  .success { background: #d4edda; color: #155724; }")
print("  .error { background: #f8d7da; color: #721c24; }")
print("</style>")
print("<script>")
print("function deleteFile(filename) {")
print("  if (!confirm('Are you sure you want to delete ' + filename + '?')) return;")
print("  ")
print("  fetch('/cgi-bin/manage_uploads.py', {")
print("    method: 'DELETE',")
print("    headers: { 'Content-Type': 'text/plain' },")
print("    body: filename")
print("  })")
print("  .then(response => {")
print("    if (response.ok) {")
print("      alert('File deleted successfully!');")
print("      window.location.reload();")
print("    } else {")
print("      alert('Failed to delete file');")
print("    }")
print("  })")
print("  .catch(error => {")
print("    alert('Error: ' + error);")
print("  });")
print("}")
print("</script>")
print("</head>")
print("<body>")
print("<h1>Uploaded Files</h1>")

# List uploaded files
if os.path.exists(uploads_path) and os.path.isdir(uploads_path):
    files = [f for f in os.listdir(uploads_path) if os.path.isfile(os.path.join(uploads_path, f))]
    
    if files:
        print("<ul class='file-list'>")
        for filename in sorted(files):
            file_path = os.path.join(uploads_path, filename)
            file_size = os.path.getsize(file_path) / 1024
            
            print(f"<li class='file-item'>")
            print(f"  <span class='file-name'>{filename}</span>")
            print(f"  <span class='file-size'>{file_size:.2f} KB</span>")
            print(f"  <button onclick='deleteFile(\"{filename}\")' class='delete-btn'>Delete</button>")
            print(f"</li>")
        print("</ul>")
    else:
        print("<p class='no-files'>No files uploaded yet.</p>")
else:
    print("<p class='no-files'>Upload directory not found.</p>")

print("</body>")
print("</html>")
