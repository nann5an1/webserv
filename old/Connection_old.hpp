#ifndef CONNECTION_OLD_HPP
#define CONNECTION_OLD_HPP

#include <ctime>
#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Handle.hpp"

class	Connection
{
	private:
		fd				_fd;
		std::string		_ip;
		int				_port;
		std::time_t		_time;
		Request			_req;
		Response		_rep;
		const Server	*_server;
		const t_location*	find_location(std::string &req_url, std::string &final_path);

	public:
		Connection();
		Connection(const Connection &other);
		Connection	&operator=(const Connection &other);
		~Connection();

		Connection(const Server *server);

		bool	request();
		bool	response();
		
		void	route();

		operator	fd() const;
		operator	std::time_t() const;

		void		set_req(Request &req);
		void		set_server(Server *server);
		std::time_t	contime() const;
};

#endif
