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

const char*	Response::build()
{
	_status = 200;
	_type = "text/plain";
	_body = "welcome from webserv!";
	_size = std::strlen(_body.c_str());
	_reply = "HTTP/1.1 " + to_string(_status) + " " + gphrase[_status] + CRLF +
			 "Content-Type: " + _type + CRLF +
			 "Content-Length: " + to_string(_size) + CRLF +
			 CRLF + _body + CRLF;
	return (*this);
}

Response::operator const char*() const
{
	return _reply.c_str();
}
