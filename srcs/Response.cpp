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
	std::cout << std::string(40, '=') << "\n" << str << std::endl;
    // Parse CGI-style response: headers then blank line then body
    // Recognized headers: Status, Content-Type, Location, Content-Length
    _status = 200;
    _type = "text/plain";
    _location.clear();
    _body.clear();

    // Split headers/body at first CRLFCRLF
    size_t sep = str.find("\r\n\r\n");
    std::string headers = sep != std::string::npos ? str.substr(0, sep) : str;
    std::string body    = sep != std::string::npos ? str.substr(sep + 4) : "";

    std::istringstream iss(headers);
    std::string line;
    while (std::getline(iss, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        if (line.empty()) continue;

        if (line.rfind("Status:", 0) == 0)
        {
            // "Status: 200 OK" or "Status: 404"
            std::istringstream ls(line.substr(7));
            int code = 200;
            ls >> code;
            if (code >= 100 && code <= 599)
                _status = code;
        }
        else if (line.rfind("Content-Type:", 0) == 0)
        {
            std::string v = line.substr(std::string("Content-Type:").size());
            // trim leading space
            while (!v.empty() && (v[0] == ' ' || v[0] == '\t')) v.erase(0,1);
            _type = v;
        }
        else if (line.rfind("Location:", 0) == 0)
        {
            std::string v = line.substr(std::string("Location:").size());
            while (!v.empty() && (v[0] == ' ' || v[0] == '\t')) v.erase(0,1);
            _location = v;
        }
        // ignore other headers here
    }

    _body = body;
    return _status;
}

const char*	Response::build()
{
    _reply = "HTTP/1.1 " + to_string(_status) + " " + gphrase[_status] + CRLF;
    if (!_location.empty())
        _reply += "Location: " + _location + CRLF;
    if (!_type.empty())
        _reply += "Content-Type: " + _type + CRLF;
    if (!_body.empty())
        _reply += "Content-Length: " + to_string(std::strlen(_body.c_str())) + CRLF;

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
