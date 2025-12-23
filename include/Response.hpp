#pragma once

#include "Server.hpp"
#include "Utils.hpp"

struct	Response
{
	int	_status;
	std::string	_reply, _body, _type, _location;

	const char*	build();
	operator	const char*() const;
};

// class	Response
// {
// 		int			_status, _size;
// 		std::string	_reply, _body, _type;
		
// 	public:
// 		Response();
// 		Response(const Response &other);
// 		Response	&operator=(const Response &other);
// 		~Response();

// 		int	set();
// 		const char*	build();
// 		operator const char*() const;
// };