#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <map>
#include <cstring>
#include <cstdlib>
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <fstream>
#include <sstream>

#define	RED	"\033[31m"
#define	RESET "\033[0m"

// TEMP~ i want to overload the sock, but not as int, 
// bcuz i rather overload int as port but not sure yet. Let me test this first. Thank you :)
struct	fd 
{
   	int fd_;
	fd();
    fd(int FD);
    operator int() const;
};

template <class CharT, class Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const fd& other)
{
	os << other.fd_;
	return (os);
}

//location scope that is to be under Server
typedef struct	s_location
{
	bool	autoindex;
	bool	get;
	bool	post;
	bool	del;
	int		return_code;
	std::string return_url;
	std::string	root;
	std::string	upload_dir;
	std::vector<std::string>	index_files;
	std::vector<std::string>	page_seq;
	std::map<std::string, std::string>	cgi;
	std::map<int, std::string> ret_pages;
}	t_location;


class Server{
    private:
		fd	_sock_fd;

		std::string _server_name;
		std::string listen_port;
		std::string listen_ip;
		std::string root;
		long long max_body_size;
		std::string location_path;
		std::map<std::string, t_location> location_map;
		std::map<int, std::string> err_pages;
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
