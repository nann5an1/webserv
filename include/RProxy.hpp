#ifndef RPROXY_HPP
#define RPROXY_HPP

#include "Utils.hpp"

class	RProxy
{
	private:
		std::string	_body;
	public:
		RProxy();
		RProxy(const RProxy &other);
		RProxy	&operator=(const RProxy &other);
		~RProxy();


		
};

#endif
