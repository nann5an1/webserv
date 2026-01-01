#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <ctime>
#include "Request.hpp"
#include "Response.hpp"
#include "Handle.hpp"
#include "IPollable.hpp"
#include "Cgi.hpp"

class Server;
class Cgi;

struct	t_reader
{
	std::string	buffer;
	std::string	header;
	std::string	body;
	bool		is_chunked;
	size_t		content_len;
	bool		read_body();
};

enum	con_state
{
	CREATED,
	READING_HEADERS,
	READING_BODY,
	PROCESSING,
	READING_RESPONSE,
	DONE,
};

class	Connection : public IPollable
{
	private:
		fd				_fd;
		const Server	*_server;

		std::string	_ip;
		int			_port;
		std::time_t	_time;

		Cgi			*_cgi;

		std::string _loc;
		con_state	_state;
		t_reader	_reader;
		Request		_req;
		Response	_rep;
		
		const t_location*	find_location(std::string &req_url, std::string &final_path, std::string &remain_path);
		void	handle(uint32_t	events);
		bool	read_header();
		bool	read_body();

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
		// std::vector<std::string> get_server_idx() const;

		operator	std::time_t() const;
		operator	fd() const;

		void		set_req(Request &req);
		void		set_server(Server *server);
		
		std::time_t	con_time() const;
};

#endif
