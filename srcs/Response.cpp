#include "Response.hpp"

Response::Response() : _status(0), _type(""), _size(0), _body(""), _reply("") {}

Response::Response(const Response &other) : _status(other._status), _type(other._type), _size(other._size), 
											_body(other._body), _reply(other._reply) {}

Response	&Response::operator=(const Response &other)
{
	if (this != &other)
	{
		_status = other._status;
		_type = other._type;
		_size = other._size;
		_body = other._body;
		_reply = other._reply;
	}
	return (*this);
}

Response::~Response() {}

void	Response::build()
{
	std::stringstream ss;
    ss << _status;
	_reply = "HTTP/1.1 " + ss.str() + " " + gphrase[_status] + CRLF +
			 "Content-Type: " + _type + CRLF +
			 "Content-Length: " + _size + CRLF +
			 CRLF + _body + CRLF;

	_reply = "HTTP/1.1 200 OK\r\n \
			  Content-Type: text/html\r\n \
			  Content-Length: 57\r\n \
			  \r\n \
			  <html><body><h1>Hello from my Webserv!</h1></body></html>";
}

Response::operator const char*() const
{
	return _reply.c_str();
}
