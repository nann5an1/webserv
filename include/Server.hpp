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

// TEMP~ i want to overload the sock, but not as int, 
// bcuz i rather overload int as port but not sure yet. Let me test this first. Thank you :)

//location scope that is to be under Server
typedef struct	s_location
{
	bool		autoindex;
	bool		get;
	bool		post;
	bool		del;
	int			return_code;
	std::string return_url;
	std::string	root;
	std::string	upload_dir;
	std::vector<std::string>	index_files;
	std::vector<std::string>	page_seq;
	std::map<std::string, std::string>	cgi;
	std::map<int, std::string> ret_pages;

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
		std::map<std::string, t_location>	_locations;
		std::map<int, std::string>			_err_pages;
    public:
        Server();
		Server(std::ifstream &file, int server_scope);
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

		// int scopeValidation(std::ifstream &file);
		int inputData(std::string &line);
		int inputLocation(std::string line, t_location &location);
		std::string trimSemiColon(std::string val);
		int validateHTTPCode(std::string &val);
		void print() const;
};

class ConfigFileError : public std::runtime_error {
public:
	ConfigFileError(); // constructor declaration
};

#endif
