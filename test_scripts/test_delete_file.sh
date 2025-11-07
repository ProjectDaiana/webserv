#!/bin/bash

# Send a DELETE request to the manage_uploads.py CGI script

FILENAME="${1}"

if [ -z "$FILENAME" ]; then
    echo "Usage: $0 <filename_to_delete>"
    echo "Example: $0 lala.jpeg"
    exit 1
fi

OUTPUT="./delete_request.bin"

echo "Creating DELETE request for: $FILENAME"

# Use Python to create the DELETE request
python3 << EOF
# Build DELETE request with filename in body
filename = '$FILENAME'
body = filename

request = (
    b"DELETE /cgi-bin/manage_uploads.py HTTP/1.1\\r\\n"
    b"Host: localhost\\r\\n"
    b"Content-Length: " + str(len(body)).encode() + b"\\r\\n"
    b"Content-Type: text/plain\\r\\n"
    b"\\r\\n"
) + body

# Save to file
with open('$OUTPUT', 'wb') as f:
    f.write(request)

print(f"DELETE request created ({len(request)} bytes)")
print(f"Filename to delete: {filename}")
EOF

echo ""
echo "✅ DELETE request file created!"
echo ""
echo "Sending to server..."
cat "$OUTPUT" | nc localhost 8080

echo ""
echo "✅ DELETE request sent!"
echo ""
