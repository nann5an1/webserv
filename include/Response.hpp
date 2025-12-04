#pragma once

#include "Server.hpp"

class	Response
{
	private:
		int			_status, _size;
		std::string	_reply, _body, _type;
		
	public:
		Response();
		Response(const Response &other);
		Response	&operator=(const Response &other);
		~Response();

		void	build();
		operator const char*() const;
};