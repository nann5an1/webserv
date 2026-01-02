#!/usr/bin/env python3

import cgi
import cgitb
cgitb.enable()  # Enable detailed error messages in the browser

# Specify the file to read
file_path = "/home/aoo/CORE/webserv/sites/tmp/"

# Start HTML output
print("Content-Type: text/html\n")
print("<html><body>")
print("<h1>File Contents:</h1>")

try:
    with open(file_path, "r") as f:
        content = f.read()
        # Escape HTML special characters
        content = content.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
        print(f"<pre>{content}</pre>")
except FileNotFoundError:
    print(f"<p>File '{file_path}' not found.</p>")
except Exception as e:
    print(f"<p>Error reading file: {e}</p>")

print("</body></html>")


struct {
	int a;
	int 4;
}

struct {

	char b;
}