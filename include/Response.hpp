#pragma once

#include "Server.hpp"

class	Response
{
	private:
		std::string	_reply;
	public:
		Response();
		Response(const Response &other);
		Response	&operator=(const Response &other);
		~Response();

		operator const char*() const;
};