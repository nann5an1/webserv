#include "Response.hpp"

// Response::Response() : _status(0), _type(""), _size(0), _body(""), _reply("") {}

// Response::Response(const Response &other) : _status(other._status), _type(other._type), _size(other._size), 
// 											_body(other._body), _reply(other._reply) {}

// Response	&Response::operator=(const Response &other)
// {
// 	if (this != &other)
// 	{
// 		_status = other._status;
// 		_type = other._type;
// 		_size = other._size;
// 		_body = other._body;
// 		_reply = other._reply;
// 	}
// 	return (*this);
// }

// Response::~Response() {}

// int	Response::set()
// {

// }

const char*	Response::build()
{
	_reply = "HTTP/1.1 " + to_string(_status) + " " + gphrase[_status] + CRLF;
	if (!_location.empty())
		_reply += "Location: " + _location + CRLF;
	if (!_type.empty())
		_reply += "Content-Type: " + _type + CRLF;
	if (!_body.empty())
		_reply += "Content-Length: " + to_string(std::strlen(_body.c_str())) + CRLF +
				  CRLF + _body + CRLF;
	// _reply = "HTTP/1.1 " + to_string(_status) + " " + gphrase[_status] + CRLF +
	// 		 "Content-Type: " + _type + CRLF +
	// 		 "Content-Length: " + to_string(std::strlen(_body.c_str())) + CRLF +
	// 		 CRLF + _body + CRLF;

	return (*this);
}

Response::operator const char*() const
{
	return (_reply.c_str());
}
