#!/usr/bin/env python3

import time
import sys

# Send minimal HTTP headers first
print("Content-Type: text/plain")
print()  # blank line

# Flush headers to the client
sys.stdout.flush()

# Infinite loop to test timeout
while True:
    print("Still running...\n", flush=True)
    time.sleep(1)

