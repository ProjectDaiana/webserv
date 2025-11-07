#!/bin/bash

# Simple DELETE request using curl

FILENAME="${1}"

if [ -z "$FILENAME" ]; then
    echo "Usage: $0 <filename_to_delete>"
    echo "Example: $0 lala.jpeg"
    exit 1
fi

echo "Sending DELETE request for: $FILENAME"
echo ""

# Send DELETE request with filename in body
curl -X DELETE \
  -H "Content-Type: text/plain" \
  -d "$FILENAME" \
  http://localhost:8080/cgi-bin/manage_uploads.py

echo ""
echo ""
echo "✅ DELETE request sent!"
