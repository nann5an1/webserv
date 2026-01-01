#ifndef IPOLLABLE_HPP
#define IPOLLABLE_HPP

#include "Utils.hpp"

struct	IPollable
{
	virtual	~IPollable() {};
	virtual void	handle(uint32_t) = 0;
};

#endif
