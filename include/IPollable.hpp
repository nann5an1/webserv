#ifndef IPOLLABLE_HPP
#define IPOLLABLE_HPP

#include "Utils.hpp"

struct	IPollable
{
	virtual	~IPollable() {};
	virtual void	handle(uint32_t) = 0;
	virtual bool	is_timeout() const = 0;
	virtual void	timeout() = 0;
	virtual void	cleanup() = 0;
};

#endif
