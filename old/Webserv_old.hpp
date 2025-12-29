#ifndef WEBSERV_OLD_HPP
#define WEBSERV_OLD_HPP


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
		std::map <fd, const Server*>	_servers_map;		
		std::map <fd, Connection>		_cons;

		fd	_ep_fd;
	
		int		server_add();
		int		create_con(const Server*);
		void	timeout();
		int		servers_start(std::set<fd>&);
		void	con_close(fd _fd);

	public:
		std::vector<Server>	_servers;	//take the sever class as type and save a collection of server
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