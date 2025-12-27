#ifndef POLLABLE_HPP
#define POLLABLE_HPP

#include "Utils.hpp"

struct	Pollable
{
	protected:
		fd	_fd;
	public:
		Pollable(fd fd_);
		virtual	~Pollable();

		virtual void	handle(uint32_t	events) = 0;

		operator	int() const;
		operator	fd() const;
};

#endif
