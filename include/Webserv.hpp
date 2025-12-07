#ifndef WEBSERV_HPP
#define WEBSERV_HPP


#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
// #include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctime>
#include <vector>
#include <map>
#include <set>
#include <exception>
#include "Server.hpp"
#include "Connection.hpp"

#define	MAX_EVENTS	1024
#define WAIT_TIME	5 // wait time: 1s

class	Webserv
{
	private:
		std::vector<Server> 		_servers;	//take the sever class as type and save a collection of servers
		std::map <fd, Connection>	_cons;

		fd	_ep_fd;
		int	_status;
	
		int		server_add(std::set<fd> &sever_fds);
		int		create_con(fd event_fd);
		void	timeout();
		int		servers_start();
		void	con_close(fd _fd);

	public:
		Webserv();
		~Webserv();
		Webserv(const Webserv &other);
		Webserv& operator=(const Webserv &other);

		void	fileParser(char *av);

		int		scopeValidation(std::ifstream &file);

		int		start();
		void	print_server_head() const;
		void	printServers() const;

	
};


class ConfigValidationError : public std::runtime_error {
public:
	ConfigValidationError(); // constructor declaration
};
#endif