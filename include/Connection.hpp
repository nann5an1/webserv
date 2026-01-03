#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <ctime>
#include "Request.hpp"
#include "Response.hpp"
#include "Handle.hpp"
#include "IPollable.hpp"
#include "Cgi.hpp"

#define CON_TIMEOUT	10

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
		fd					_fd;
		const Server		*_server;

		std::string			_ip;
		int					_port;
		std::time_t			_time;

		Cgi					*_cgi;

		const t_location	*_location;
		std::string 		_loc;
		con_state			_state;
		t_reader			_reader;
		Request				_req;
		Response			_rep;
		
		const t_location*	find_location(std::string &req_url, std::string &final_path, std::string &remain_path);
		void	handle(uint32_t	events);
		bool	read_header();
		// bool	read_body();

		Connection(const Connection &other);
		Connection	&operator=(const Connection &other);
		
	public:
		Connection();
		~Connection();

		Connection(const Server *server);

		bool	request();
		void	route();
		void	handle_error();
		bool	response();
		void	cleanup();
		// std::vector<std::string> get_server_idx() const;

		operator	std::time_t() const;
		operator	fd() const;

		void		set_req(Request &req);
		void		set_server(Server *server);
		
		bool		is_timeout() const;
		void		timeout();
		std::time_t	get_time() const;
};

#endif
