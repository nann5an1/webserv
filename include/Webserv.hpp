#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <set>
#include <exception>
#include <fstream>
#include <iomanip>
#include <string>
#include <ctime>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "Utils.hpp"
#include "Server.hpp"
#include "Pollable.hpp"
#include "Epoll.hpp"

#define	MAX_EVENTS	1024
#define WAIT_TIME	5 // wait time in sec

class	Webserv
{
	private:
		bool		servers_start();
		bool		server_add();
		// int		create_con(const Server*);
		// void	timeout();
		// void	close_con(fd _fd);

	public:
		std::vector<Server>	_servers;

		Webserv();
		Webserv(const Webserv &other);
		Webserv& operator=(const Webserv &other);
		~Webserv();

		void	fileParser(char *av);

		int		scopeValidation(std::ifstream &file);
		void	print_server_head() const;
		void	printServers() const;

		int		start();
};

#endif