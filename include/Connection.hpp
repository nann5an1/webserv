#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <ctime>
#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Handle.hpp"
#include "Pollable.hpp"

enum state
{
	req_read;
	req_body;
	req_
};

class	Connection : public Pollable
{
	private:
		std::string	_ip;
		int			_port;
		std::time_t	_time;
		Request		_req;
		Response	_rep;
		const Server	*_server;
		const t_location*	find_location(std::string &req_url, std::string &final_path, std::string &remain_path);
		void	handle(uint32_t	events);

	public:
		Connection();
		Connection(const Connection &other);
		Connection	&operator=(const Connection &other);
		~Connection();

		Connection(const Server *server);

		bool	request();
		bool	response();
		void	route();
		void	cleanup();

		operator	std::time_t() const;

		void		set_req(Request &req);
		void		set_server(Server *server);
		
		std::time_t	con_time() const;
};

#endif
