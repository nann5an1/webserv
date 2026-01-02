#include "Response.hpp"

Response::Response() : _status(200), _type("text/plain"), _body(""), _reply(""), _location("") {}

Response::Response(const Response &other) : _status(other._status), _type(other._type), 
											_body(other._body), _reply(other._reply),  _location(other._location) {}

Response	&Response::operator=(const Response &other)
{
	if (this != &other)
	{
		_status = other._status;
		_type = other._type;
		_location = other._location;
		_body = other._body;
		_reply = other._reply;
	}
	return (*this);
}

Response::~Response() {}

int	Response::cgi_handle(const std::string &str)
{
    // ...existing code...

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
        }
    }
    _body = body;
    return (_status);
}

const char*	Response::build()
{
    _reply = "HTTP/1.1 " + to_string(_status) + " " + gphrase[_status] + CRLF;
    if (!_location.empty())
        _reply += "Location: " + _location + CRLF;
    if (!_type.empty())
        _reply += "Content-Type: " + _type + CRLF;
    if (!_body.empty())
        _reply += "Content-Length: " + to_string(_body.size()) + CRLF;

    // End of headers
    _reply += CRLF;
    // Optional body
    if (!_body.empty())
        _reply += _body;

    return _reply.c_str();  // FIX: return backing c_str
}

Response::operator const char*() const
{
	return (_reply.c_str());
}
