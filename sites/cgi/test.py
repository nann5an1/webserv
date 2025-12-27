#!/usr/bin/env python3
# Make sure the script is executable: chmod +x test.py

import os
import sys
import cgi

# Print required CGI headers
print("Content-Type: text/html")
print()  # End of headers

# Begin HTML output
print("<html><body>")

# Show request method
method = os.environ.get("REQUEST_METHOD", "UNKNOWN")
print(f"<h3>Request Method: {method}</h3>")

# Handle GET query parameters
if method == "GET":
    query = os.environ.get("QUERY_STRING", "")
    print(f"<p>Query String: {query}</p>")

# Handle POST form data
elif method == "POST":
    # Read content length
    try:
        content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    except ValueError:
        content_length = 0

    post_data = sys.stdin.read(content_length)
    print(f"<p>POST Data:</p><pre>{cgi.escape(post_data)}</pre>")

print("<p>CGI script executed successfully!</p>")
print("</body></html>")
