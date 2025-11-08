#!/bin/bash

# Test script for error codes with colors
# Make sure your server is running before executing this

# Colors (matching errors.hpp)
CLR_RESET="\033[0m"
CLR_RED="\033[1;31m"
CLR_GREEN="\033[1;32m"
CLR_YELLOW="\033[1;33m"
CLR_CYAN="\033[1;36m"
CLR_MAGENTA="\033[1;35m"
CLR_WHITE="\033[1;37m"

echo -e "${CLR_MAGENTA}=========================================${CLR_RESET}"
echo -e "${CLR_CYAN}🎃 Testing Error Codes 🎃${CLR_RESET}"
echo -e "${CLR_MAGENTA}=========================================${CLR_RESET}"
echo ""

# Test 1: Valid request (should return 200)
echo -e "${CLR_YELLOW}Test 1:${CLR_RESET} Valid request - ${CLR_CYAN}GET /${CLR_RESET}"
# curl -s (silent), -o /dev/null (discard body), -w "%{http_code}" (print only status code)
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:8080/)
if [ "$RESPONSE" = "200" ]; then
    echo -e "${CLR_GREEN}✓ PASS: Got $RESPONSE${CLR_RESET}"
else
    echo -e "${CLR_RED}✗ FAIL: Expected 200, got $RESPONSE${CLR_RESET}"
fi
echo ""

# Test 2: URI too long (should return 414 with MAX_URI_LEN=8000)
echo -e "${CLR_YELLOW}Test 2:${CLR_RESET} URI too long - ${CLR_CYAN}/<8001 chars>${CLR_RESET}"
# printf 'a%.0s' {1..8001} creates a string of 8001 'a' characters
LONG_URI=$(printf 'a%.0s' {1..8001})
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" "http://localhost:8080/$LONG_URI")
if [ "$RESPONSE" = "414" ]; then
    echo -e "${CLR_GREEN}✓ PASS: Got $RESPONSE${CLR_RESET}"
else
    echo -e "${CLR_RED}✗ FAIL: Expected 414, got $RESPONSE${CLR_RESET}"
fi
echo ""

# Test 3: Method not allowed (should return 405)
echo -e "${CLR_YELLOW}Test 3:${CLR_RESET} Method not allowed - ${CLR_CYAN}PUT${CLR_RESET}"
# -X PUT specifies the HTTP method
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -X PUT http://localhost:8080/)
if [ "$RESPONSE" = "405" ]; then
    echo -e "${CLR_GREEN}✓ PASS: Got $RESPONSE${CLR_RESET}"
else
    echo -e "${CLR_RED}✗ FAIL: Expected 405, got $RESPONSE${CLR_RESET}"
fi
echo ""

# Test 4: Method not allowed (should return 405)
echo -e "${CLR_YELLOW}Test 4:${CLR_RESET} Method not allowed - ${CLR_CYAN}PATCH${CLR_RESET}"
# -X PATCH specifies the HTTP method
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -X PATCH http://localhost:8080/)
if [ "$RESPONSE" = "405" ]; then
    echo -e "${CLR_GREEN}✓ PASS: Got $RESPONSE${CLR_RESET}"
else
    echo -e "${CLR_RED}✗ FAIL: Expected 405, got $RESPONSE${CLR_RESET}"
fi
echo ""

# Test 5: Invalid method (should return 405 or 400)
echo -e "${CLR_YELLOW}Test 5:${CLR_RESET} Invalid method - ${CLR_CYAN}INVALID${CLR_RESET}"
# -X INVALID sends a non-standard HTTP method
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -X INVALID http://localhost:8080/)
if [ "$RESPONSE" = "405" ] || [ "$RESPONSE" = "400" ]; then
    echo -e "${CLR_GREEN}✓ PASS: Got $RESPONSE${CLR_RESET}"
else
    echo -e "${CLR_RED}✗ FAIL: Expected 405 or 400, got $RESPONSE${CLR_RESET}"
fi
echo ""

# Test 6: Multiple ? in URI (should return 400)
echo -e "${CLR_YELLOW}Test 6:${CLR_RESET} Multiple ? in URI - ${CLR_CYAN}/?a=1?b=2${CLR_RESET}"
# URIs should only have one '?' character (malformed URI)
RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" "http://localhost:8080/?a=1?b=2")
if [ "$RESPONSE" = "400" ]; then
    echo -e "${CLR_GREEN}✓ PASS: Got $RESPONSE${CLR_RESET}"
else
    echo -e "${CLR_RED}✗ FAIL: Expected 400, got $RESPONSE${CLR_RESET}"
fi
echo ""

# Test 7: POST without Content-Length or Transfer-Encoding (should return 411)
echo -e "${CLR_YELLOW}Test 7:${CLR_RESET} POST without Content-Length - ${CLR_CYAN}411 Length Required${CLR_RESET}"
# nc (netcat) sends raw data without automatic headers (unlike curl)
# -w 1 sets 1 second timeout, grep finds the HTTP response line, awk extracts 2nd field (status code)
RESPONSE=$({
    echo -e "POST / HTTP/1.1\r"
    echo -e "Host: localhost:8080\r"
    echo -e "\r"
    echo -e "test body data\r"
} | nc -w 1 localhost 8080 2>&1 | grep "HTTP/1.1" | awk '{print $2}')
if [ "$RESPONSE" = "411" ]; then
    echo -e "${CLR_GREEN}✓ PASS: Got $RESPONSE${CLR_RESET}"
else
    echo -e "${CLR_RED}✗ FAIL: Expected 411, got $RESPONSE${CLR_RESET}"
fi
echo ""


# Test 8: Request timeout (should return 408)
echo -e "${CLR_YELLOW}Test 9:${CLR_RESET} Request timeout - ${CLR_CYAN}408 Request Timeout${CLR_RESET}"
echo -e "${CLR_WHITE}(Timeout set to 10s - sending incomplete request and waiting)${CLR_RESET}"
# Send incomplete headers (no final \r\n) and wait longer than CLIENT_INACTIVITY_TIMEOUT
RESPONSE=$({
    printf "GET / HTTP/1.1\r\n"
    printf "Host: localhost:8080\r\n"
    # Don't send final \r\n to complete headers - server should timeout
    sleep 12  # Wait longer than 10 second timeout
} | nc localhost 8080 2>&1 | grep "HTTP/1.1" | awk '{print $2}')
if [ "$RESPONSE" = "408" ]; then
    echo -e "${CLR_GREEN}✓ PASS: Got $RESPONSE${CLR_RESET}"
else
    echo -e "${CLR_RED}✗ FAIL: Expected 408, got $RESPONSE${CLR_RESET}"
fi
echo ""

echo -e "${CLR_MAGENTA}=========================================${CLR_RESET}"
echo -e "${CLR_GREEN}✨ Tests Complete ✨${CLR_RESET}"
echo -e "${CLR_MAGENTA}=========================================${CLR_RESET}"
