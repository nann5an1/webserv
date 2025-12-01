#pragma once

#include <ctime>
#include "Server.hpp"

class	Connection
{
	private:
		fd	_fd;
		std::time_t	_time;
	public:
		Connection();
		Connection(const Connection &other);
		Connection	&operator=(const Connection &other);
		~Connection();

		Connection(fd server_fd);

		bool	getRequest();
		operator	fd() const;
		operator	std::time_t() const;
};