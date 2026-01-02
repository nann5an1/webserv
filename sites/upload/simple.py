#!/usr/bin/env python3
# Make sure the script is executable: chmod +x test.py

import os
import sys
import html
import cgi
import cgitb
import json
import urllib.parse
from http import cookies
from datetime import datetime

# Helpful CGI error reports (prints a readable traceback as HTML)
cgitb.enable()

def env(name, default=""):
    return os.environ.get(name, default)

def h(s):
    return html.escape("" if s is None else str(s), quote=True)

def parse_query_string(qs: str):
    # returns dict[str, list[str]]
    return urllib.parse.parse_qs(qs or "", keep_blank_values=True)

def read_stdin_bytes(n: int) -> bytes:
    if n <= 0:
        return b""
    return sys.stdin.buffer.read(n)

def parse_urlencoded(body: bytes):
    try:
        text = body.decode("utf-8", errors="replace")
    except Exception:
        text = str(body)
    return urllib.parse.parse_qs(text, keep_blank_values=True), text

def parse_multipart_with_fieldstorage():
    # FieldStorage reads from stdin and uses env (CONTENT_TYPE/LENGTH)
    # NOTE: creating FieldStorage consumes stdin, so only call once.
    form = cgi.FieldStorage()
    fields = {}
    files = []
    for key in form.keys():
        item = form[key]
        items = item if isinstance(item, list) else [item]
        for it in items:
            if it.filename:
                data = it.file.read()
                files.append({
                    "name": key,
                    "filename": it.filename,
                    "content_type": it.type,
                    "size": len(data),
                    "first_bytes_hex": data[:32].hex(),
                })
            else:
                fields.setdefault(key, []).append(it.value)
    return fields, files

# --- Collect request basics ---
method = env("REQUEST_METHOD", "UNKNOWN")
qs = env("QUERY_STRING", "")
content_type = env("CONTENT_TYPE", "")
try:
    content_length = int(env("CONTENT_LENGTH", "0") or "0")
except ValueError:
    content_length = 0

# --- Cookies ---
cookie_in = cookies.SimpleCookie()
raw_cookie = env("HTTP_COOKIE", "")
if raw_cookie:
    cookie_in.load(raw_cookie)

# Set a cookie back (demonstrates response headers)
cookie_out = cookies.SimpleCookie()
cookie_out["cgi_test"] = "1"
cookie_out["cgi_test"]["path"] = "/"
cookie_out["cgi_test"]["max-age"] = "3600"
# cookie_out["cgi_test"]["httponly"] = True  # uncomment if desired

# --- Parse inputs (GET + POST) ---
get_params = parse_query_string(qs)
post_info = {
    "content_type": content_type,
    "content_length": content_length,
    "raw_body_preview": "",
    "raw_body_bytes": 0,
    "urlencoded": {},
    "multipart_fields": {},
    "multipart_files": [],
}

# For POST/PUT/PATCH, support urlencoded + multipart + raw
if method in ("POST", "PUT", "PATCH"):
    if content_type.startswith("application/x-www-form-urlencoded"):
        body = read_stdin_bytes(content_length)
        parsed, text = parse_urlencoded(body)
        post_info["raw_body_preview"] = text[:2000]
        post_info["raw_body_bytes"] = len(body)
        post_info["urlencoded"] = parsed

    elif content_type.startswith("multipart/form-data"):
        # Use FieldStorage to parse multipart + uploaded files
        fields, files = parse_multipart_with_fieldstorage()
        post_info["multipart_fields"] = fields
        post_info["multipart_files"] = files

    else:
        # Unknown/other types: read raw body
        body = read_stdin_bytes(content_length)
        post_info["raw_body_bytes"] = len(body)
        try:
            post_info["raw_body_preview"] = body.decode("utf-8", errors="replace")[:2000]
        except Exception:
            post_info["raw_body_preview"] = repr(body[:2000])

# --- Response headers ---
print("Content-Type: text/html; charset=utf-8")
# Example custom header:
print("X-CGI-Test: ok")
# Cookie header(s):
for morsel in cookie_out.values():
    print(morsel.OutputString())
print()  # end headers

# --- HTML body ---
print("<!doctype html>")
print("<html><head><meta charset='utf-8'><title>CGI Test</title></head><body>")
print("<h2>CGI Feature Test</h2>")

print("<h3>Request</h3>")
print("<ul>")
print(f"<li><b>Time:</b> {h(datetime.utcnow().isoformat())}Z</li>")
print(f"<li><b>Method:</b> {h(method)}</li>")
print(f"<li><b>Path Info:</b> {h(env('PATH_INFO'))}</li>")
print(f"<li><b>Script Name:</b> {h(env('SCRIPT_NAME'))}</li>")
print(f"<li><b>Query String:</b> {h(qs)}</li>")
print(f"<li><b>Content-Type:</b> {h(content_type)}</li>")
print(f"<li><b>Content-Length:</b> {h(content_length)}</li>")
print("</ul>")

print("<h3>GET parameters</h3>")
print("<pre>" + h(json.dumps(get_params, indent=2, sort_keys=True)) + "</pre>")

print("<h3>POST/Body</h3>")
print("<pre>" + h(json.dumps(post_info, indent=2, sort_keys=True)) + "</pre>")

print("<h3>Incoming Cookies</h3>")
incoming = {k: cookie_in[k].value for k in cookie_in.keys()}
print("<pre>" + h(json.dumps(incoming, indent=2, sort_keys=True)) + "</pre>")

print("<h3>Selected Headers (from environment)</h3>")
header_like = {}
for k, v in sorted(os.environ.items()):
    if k.startswith("HTTP_") or k in ("CONTENT_TYPE", "CONTENT_LENGTH"):
        header_like[k] = v
print("<pre>" + h(json.dumps(header_like, indent=2, sort_keys=True)) + "</pre>")

print("<hr>")
print("<h3>Try it</h3>")
print("<ul>")
print("<li>GET: <code>?a=1&amp;a=2&amp;empty=&amp;space=hello+world</code></li>")
print("<li>POST urlencoded: <code>curl -i -X POST -d 'x=1&y=hello' http://HOST/cgi/test.py</code></li>")
print("<li>POST multipart (file upload): <code>curl -i -X POST -F 't=123' -F 'f=@/etc/hosts' http://HOST/cgi/test.py</code></li>")
print("<li>Custom header: <code>curl -i -H 'X-Demo: 123' http://HOST/cgi/test.py</code></li>")
print("</ul>")

print("</body></html>")
