#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <ctime>
#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"

class	Connection
{
	private:
		fd	_fd;
		std::time_t	_time;
		// Request	_req;
		Response	_rep;
		Server		_server;
	public:
		Connection();
		Connection(const Connection &other);
		Connection	&operator=(const Connection &other);
		~Connection();

		Connection(const Server &server);

		bool	request();
		bool	response();

		operator	fd() const;
		operator	std::time_t() const;

		std::time_t	contime() const;
};

#endif
