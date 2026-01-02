#ifndef SERVER_HPP
#define SERVER_HPP

#include <fstream>
#include <cstdlib>
#include <ctime>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "IPollable.hpp"
#include "Epoll.hpp"
#include "Utils.hpp"


class	Connection;

typedef struct	s_location
{
	bool		autoindex;
	int			methods;
	int			r_status;
	std::string r_url;
	std::string	root;
	std::string	upload_dir;
	std::vector<std::string>	index_files;
	std::map<std::string, std::string>	cgi;
	std::map<int, std::string> ret_pages;
	std::map<int, std::string> err_pages;
	std::string	rproxy;
}	t_location;

class	Server : public IPollable
{
	private:
		fd			_fd;
		std::string _name;
		std::string _ip;
		std::string _port;
		std::string _root;
		long long	_max_size;
		int			_r_status;
		std::string	_r_url;
		std::time_t	_time;
		std::vector<std::string> _server_idx;
		std::map<int, std::string>			_err_pages;
		std::map<std::string, t_location>	_locations;
	
		int	parse_return(std::stringstream&, int&, std::string&);
		int	parse_err_pages(std::stringstream&, std::map<int,std::string>& err_pg_container);
		void	handle(uint32_t events);

	public:
		Server();
		Server(const Server &other);
		Server	&operator=(const Server &other);
		~Server();

		Server(std::ifstream &file);

		static std::string	trimSemiColon(std::string val);
		static int validateHTTPCode(int &code);

		int inputData(std::string &line);
		int inputLocation(std::string line, t_location &location);

		void	print() const;
		int		start();
		bool	is_timeout() const;
		void	timeout();
		void	cleanup();

		operator fd() const;
		operator int() const;
		operator std::string() const;

		std::string	name() const;
		std::string	ip() const;
		std::string	port() const;
		std::string	root() const;
		int			r_status() const;
		std::string	r_url() const;
		long long	max_size() const;
		const std::map<int, std::string>& err_pages() const;
		
		std::vector<std::string> server_idx() const;

		// std::vector<std::string> server_idx() const;
		const std::map<std::string, t_location>&	locations() const;
};

#endif
