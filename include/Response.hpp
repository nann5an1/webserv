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


		size_t headerSize() const;      // returns size of headers (_reply)
		const char* bodyData() const;   // pointer to body data (_body)
		size_t bodySize() const;        // size of body in bytes

		int	cgi_handle(const std::string &str);
		const char*	build();
		operator const char*() const;
};