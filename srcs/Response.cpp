#include "Response.hpp"
#include <sstream>
#include <iostream>
#include <cstring>

Response::Response() : _status(200), _type("text/plain"), _body(""), _reply(""), _location("") {}

Response::Response(const Response &other) : _status(other._status), _type(other._type), 
											_body(other._body), _reply(other._reply),  _location(other._location) {}

Response &Response::operator=(const Response &other)
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

int Response::cgi_handle(const std::string &str)
{
	std::cout << std::string(40, '=') << "\n" << str << std::endl;

	_status = 200;
	_type.clear();
	_location.clear();
	_body.clear();

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
			std::istringstream ls(line.substr(7));
			int code = 200;
			ls >> code;
			if (code >= 100 && code <= 599)
				_status = code;
		}
		else if (line.rfind("Content-Type:", 0) == 0)
		{
			std::string v = line.substr(std::string("Content-Type:").size());
			while (!v.empty() && (v[0] == ' ' || v[0] == '\t')) v.erase(0,1);
			_type = v;
		}
		else if (line.rfind("Location:", 0) == 0)
		{
			std::string v = line.substr(std::string("Location:").size());
			while (!v.empty() && (v[0] == ' ' || v[0] == '\t')) v.erase(0,1);
			_location = v;
		}
	}
	_body = body;
	return _status;
}

const char* Response::build()
{
	_reply.clear();

    _reply = "HTTP/1.1 " + to_string(_status) + " " + gphrase[_status] + CRLF;


	if (!_location.empty())
		_reply += "Location: " + _location + CRLF;

	if (!_type.empty())
		_reply += "Content-Type: " + _type + CRLF;

	// Always use .size(), never strlen()
	_reply += "Content-Length: " + to_string(_body.size()) + CRLF;

	// End of headers
	_reply += CRLF;

	// Do NOT append _body here! Send body separately
	return _reply.c_str();
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
	return _reply.c_str();
}
