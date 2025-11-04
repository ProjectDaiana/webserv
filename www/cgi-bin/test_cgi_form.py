#!/usr/bin/env python3
import cgi

#print("Content-Type: text/html")
#print("")  # blank line

form = cgi.FieldStorage()
name = form.getvalue("name", "Anonymous")
age = form.getvalue("age", "unknown")

print("<html>")
print("<head><title>Greeting</title><link rel='icon' href='data:,'></head>")
print("<body>")
print(f"<h1>Hello, {name}! You are {age} years old.</h1>")
print("</body></html>")
