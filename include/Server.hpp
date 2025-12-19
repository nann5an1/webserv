#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstdlib>

#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <fstream>

#include "Utils.hpp"

typedef struct	s_location
{
	bool		autoindex;
	int			methods;
	int			r_status;
	std::string r_url;
	std::string	root;
	std::string	upload_dir;
	std::vector<std::string>	index_files;
	std::vector<std::string>	page_seq;
	std::map<std::string, std::string>	cgi;
	std::map<int, std::string> ret_pages;
	std::map<int, std::string> err_pages;
	std::string	rproxy;
}	t_location;


class Server{
    private:
		fd			_sock_fd;
		std::string _name;
		std::string _port;
		std::string _ip;
		std::string _root;
		long long	_max_size;
		std::string location_path;
		std::string	_r_url;
		int			_r_status;
		std::map<std::string, t_location>	_locations;
		std::map<int, std::string>			_err_pages;

		int	parse_err_pages(std::stringstream&, std::map<int,std::string>&);
		int	parse_return(std::stringstream&, int&, std::string&);
    public:
        Server();
		Server(std::ifstream &file);
        Server(const Server &other);
        ~Server();
        Server &operator=(const Server &other);
        int	start();

		operator fd() const;
		operator int() const;
		operator std::string() const;

		std::string	port() const;
		std::string	ip() const;
		std::string	name() const;
		std::string	root() const;
		std::string	r_url() const;
		int			r_status() const;
		const std::map<std::string, t_location>&	locations() const;

		// int scopeValidation(std::ifstream &file);
		int inputData(std::string &line);
		int inputLocation(std::string line, t_location &location);
		std::string trimSemiColon(std::string val);
		static int validateHTTPCode(int &code);
		void	print() const;

};

class ConfigFileError : public std::runtime_error {
public:
	ConfigFileError(); // constructor declaration
};

#endif
