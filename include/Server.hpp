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

// TEMP: i want to overload the sock, but not as int, 
// bcuz i rather overload int as port but not sure yet. Let me test this first. Thank you :)
struct	fd 
{
    int FD;
	fd();
    fd(int fd_);
    operator int() const;
};

template <class CharT, class Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const fd& fd_)
{
	os << fd_.FD;
	return (os);
}



class Server{
    private:
		std::string server_name;
		std::string listen_port;
		std::string	listen_ip;
		long long max_body_size;
		// std::map<std::string, t_location> location_map;
		std::map<int, std::string> err_pages;
		fd	_sock_fd;
    public:
        Server();
        Server(const Server &other);
        ~Server();
        Server &operator=(const Server &other);
        int	start();

		operator fd() const;
		operator int() const;
		operator std::string() const;
};

#endif