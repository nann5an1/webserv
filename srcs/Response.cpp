#include "Response.hpp"

Response::Response() : _reply("test response!") {}

Response::Response(const Response &other) : _reply(other._reply) {}

Response	&Response::operator=(const Response &other)
{
	if (this != &other)
		_reply = other._reply;
	return (*this);
}

Response::~Response() {}

Response::operator const char*() const
{
	return _reply.c_str();
}
