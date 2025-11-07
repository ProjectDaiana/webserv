#!/bin/bash

# Directory containing invalid config files
CONFIG_DIR="config/invalid"

# Your webserv binary
SERVER_BIN="./webserv"

# ANSI color codes for pretty output
GREEN="\033[1;32m"
RED="\033[1;31m"
YELLOW="\033[1;33m"
RESET="\033[0m"

echo "=== 🚧 Testing invalid configuration files ==="
echo

# Exit code summary counter
total=0
passed=0

for conf in "$CONFIG_DIR"/*.conf; do
    ((total++))
    conf_name=$(basename "$conf")
    echo -e "${YELLOW}Testing:${RESET} $conf_name"

    # Run webserv with this config and capture output and exit status
    output=$($SERVER_BIN "$conf" 2>&1)
    status=$?

    # Expect failure (non-zero exit)
    if [ $status -ne 0 ]; then
        echo -e "  ${GREEN}OK${RESET} - correctly failed"
        ((passed++))
    else
        echo -e "  ${RED}FAIL${RESET} - did NOT fail as expected"
    fi

    # Optional: print the first line of the error
    echo "  → $(echo "$output" | head -n 1)"
    echo
done

echo "=== ✅ Summary: $passed / $total tests passed ==="

