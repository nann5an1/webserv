#pragma once

#include "Server.hpp"
#include "Utils.hpp"



class	Response
{	
	public:
		int	_status;
		std::string	_reply, _body, _type, _location;
		Response();
		Response(const Response &other);
		Response	&operator=(const Response &other);
		~Response();

		const char*	build();
		operator const char*() const;
};