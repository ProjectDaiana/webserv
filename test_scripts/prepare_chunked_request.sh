#!/bin/bash

# Create a chunked HTTP request file from an image that you can send via telnet/nc

IMAGE="${1:-kitty.jpg}"

if [ ! -f "$IMAGE" ]; then
    echo "Usage: $0 <image_file>"
    exit 1
fi

OUTPUT="./chunked_request.bin"

echo "Creating chunked request from: $IMAGE"

# Use Python to create the binary request file
python3 << EOF
import sys

# Read image
with open('$IMAGE', 'rb') as f:
    data = f.read()

print(f"Image size: {len(data)} bytes")

# Create chunks
chunk_size = 512
chunks = []
pos = 0

while pos < len(data):
    chunk = data[pos:pos+chunk_size]
    chunks.append(f"{len(chunk):x}\\r\\n".encode() + chunk + b"\\r\\n")
    pos += chunk_size

chunks.append(b"0\\r\\n\\r\\n")

print(f"Created {len(chunks)-1} chunks")

# Build request
request = (
    b"POST /cgi-bin/show_image.py HTTP/1.1\\r\\n"
    b"Host: localhost\\r\\n"
    b"Transfer-Encoding: chunked\\r\\n"
    b"Content-Type: image/jpeg\\r\\n"
    b"\\r\\n"
) + b"".join(chunks)

# Save to file
with open('$OUTPUT', 'wb') as f:
    f.write(request)

print(f"Saved to: $OUTPUT ({len(request)} bytes)")
EOF

echo ""
echo "✅ Chunked request file created!"
echo ""
echo "Sending to server..."
cat "$OUTPUT" | nc localhost 8080 > /dev/null

echo ""
echo "✅ Image uploaded!"
echo "View at: http://localhost:8080/cgi-bin/show_image.py"
echo ""
# xdg-open "http://localhost:8080/cgi-bin/show_image.py" 2>/dev/null &
