#include "Response.hpp"
#include <sstream>
#include <iostream>
#include <cstring>

Response::Response() : _status(200), _reply(""), _body(""), _type("text/plain"), _location("")  {}

Response::Response(const Response &other) : _status(other._status), _reply(other._reply), _body(other._body),
											_type(other._type), _location(other._location) {}

Response &Response::operator=(const Response &other)
{
	if (this != &other)
	{
		_status = other._status;
		_reply = other._reply;
		_body = other._body;
		_type = other._type;
		_location = other._location;
	}
	return (*this);
}

Response::~Response() {}


static bool decode_chunked_body(const std::string &in, std::string &out)
{
    out.clear();
    size_t pos = 0;

    while (pos < in.size())
    {
        // read chunk-size line
        size_t line_end = in.find("\r\n", pos);
        size_t line_skip = 2;
        if (line_end == std::string::npos)
        {
            line_end = in.find("\n", pos);
            line_skip = 1;
        }
        if (line_end == std::string::npos)
            return false;

        std::string line = in.substr(pos, line_end - pos);
        pos = line_end + line_skip;

        // ignore chunk extensions: "HEX;ext=..."
        size_t semi = line.find(';');
        if (semi != std::string::npos)
            line.erase(semi);

        // trim spaces
        while (!line.empty() && (line[0] == ' ' || line[0] == '\t'))
            line.erase(0, 1);
        while (!line.empty() && (line[line.size() - 1] == ' ' || line[line.size() - 1] == '\t'))
            line.erase(line.size() - 1, 1);

        char *endp = NULL;
        unsigned long chunk_sz = std::strtoul(line.c_str(), &endp, 16);
        if (endp == line.c_str())
            return false;

        if (chunk_sz == 0)
        {
            // Optional trailers exist; we can ignore them because we buffer everything anyway.
            return true;
        }

        if (pos + chunk_sz > in.size())
            return false;

        out.append(in, pos, chunk_sz);
        pos += chunk_sz;

        // expect CRLF (or LF) after chunk data
        if (pos < in.size() && in.compare(pos, 2, "\r\n") == 0)
            pos += 2;
        else if (pos < in.size() && in[pos] == '\n')
            pos += 1;
        else
            return false;
    }
    return false;
}

int	Response::cgi_handle(const std::string &str)
{
    // Split headers/body at first blank line (CRLFCRLF or LFLF)
    size_t sep = str.find("\r\n\r\n");
    size_t skip = 4;
    if (sep == std::string::npos)
    {
        sep = str.find("\n\n");
        skip = 2;
    }

    // If no separator, treat entire output as body
    std::string headers;
    std::string body;
    if (sep == std::string::npos)
    {
        headers.clear();
        body = str;
    }
    else
    {
        headers = str.substr(0, sep);
        body = str.substr(sep + skip);
    }

	bool	cgi_chunked = false;

    // Only parse headers if we actually have some
    if (!headers.empty())
    {
        std::istringstream iss(headers);
        std::string line;
        while (std::getline(iss, line))
        {
            if (!line.empty() && line[line.size() - 1] == '\r')
                line.erase(line.size() - 1);
            if (line.empty())
                continue;

            if (line.rfind("Status:", 0) == 0)
            {
                std::istringstream ls(line.substr(7));
                int code = 200;
                ls >> code;
                if (code >= 100 && code <= 599)
                    _status = code;
		    }
            else if (line.rfind("Content-Type:", 0) == 0)
            {
                std::string v = line.substr(std::string("Content-Type:").size());
                while (!v.empty() && (v[0] == ' ' || v[0] == '\t')) v.erase(0, 1);
                    _type = v;
            }
            else if (line.rfind("Location:", 0) == 0)
            {
                std::string v = line.substr(std::string("Location:").size());
                while (!v.empty() && (v[0] == ' ' || v[0] == '\t')) v.erase(0, 1);
                    _location = v;
            }
			else if (line.rfind("Transfer-Encoding:", 0) == 0)
			{
				if (line.find("chunked") != std::string::npos)
					cgi_chunked = true;
			}
	    }
    }
    if (cgi_chunked)
    {
        std::string decoded;
        if (!decode_chunked_body(body, decoded))
            return (_status = 502);
        _body = decoded;
    }
    else
        _body = body;
    return (_status);
}

std::string	Response::build()
{
	_reply.clear();
    _reply = "HTTP/1.1 " + to_string(_status) + " " + gphrase[_status] + CRLF;
    if (!_location.empty())
        _reply += "Location: " + _location + CRLF;
    if (!_type.empty())
        _reply += "Content-Type: " + _type + CRLF;
    if (!_body.empty())
	{
        _reply += "Content-Length: " + to_string(_body.size()) + CRLF;
	}
	_reply += CRLF;

	// Do NOT append _body here! Send body separately
	_reply += _body;

	return _reply;
}

// Helper to get header size
size_t Response::headerSize() const
{
	return _reply.size();
}

// Helper to get body data pointer
const char* Response::bodyData() const
{
	return _body.data();
}

// Helper to get body size
size_t Response::bodySize() const
{
	return _body.size();
}

Response::operator const char*() const
{
	return (_reply.c_str());
}
