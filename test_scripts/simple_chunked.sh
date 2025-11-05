#!/bin/bash

# Simple chunked POST - manual chunk sizes
# Usage: ./simple_chunked.sh "Your text here"

PORT="8080"
HOST="localhost"
TEXT="${1:-this is the text, of this file, wooo!}"

echo "Sending chunked POST request..."

# Calculate actual chunk size in hex
CHUNK_LEN=${#TEXT}
CHUNK_HEX=$(printf "%x" $CHUNK_LEN)

# Send the request
(
  echo -ne "POST /upload HTTP/1.1\r\n"
  echo -ne "Host: $HOST\r\n"
  echo -ne "User-Agent: Arduino\r\n"
  echo -ne "Accept: */*\r\n"
  echo -ne "Transfer-Encoding: chunked\r\n"
  echo -ne "\r\n"
  
  # First chunk with calculated size
  echo -ne "$CHUNK_HEX\r\n"
  echo -ne "$TEXT\r\n"
  
  # Terminating chunk
  echo -ne "0\r\n"
  echo -ne "\r\n"
  
  sleep 0.5
) | nc $HOST $PORT

echo ""
echo "Done! Check www/html/uploads/ for the uploaded file"
