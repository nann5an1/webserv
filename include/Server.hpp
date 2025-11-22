#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <map>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
// #include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
// #include <sys/epoll.h>

#define	RED	"\033[31m"
#define	RESET "\033[0m"

class Server{
    private:
		std::string server_name;
		std::string listen_port;
		std::string	listen_ip;
		long long max_body_size;
		std::map<std::string, t_location> location_map;
		std::map<int, std::string> err_pages;
		int _sock_fd; //endpoint for communication
    public:
        Server();
        Server(const Server &other);
        ~Server();
        Server &operator=(const Server &other);
        int	start();
};

#endif