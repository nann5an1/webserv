#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <map>
#include <exception>


typedef struct	s_location
{
	// std::string	path;
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

class Webserv{
	private:
		std::string server_name;
		std::string listen_port;
		std::string listen_ip;
		long long max_body_size;
		std::string location_path;
		std::map<std::string, t_location> location_map;
		std::map<int, std::string> err_pages;
	public:
		Webserv();
		Webserv(char *av); //take the av(filename) like this
		~Webserv();
		Webserv(const Webserv &other);
		Webserv& operator=(const Webserv &other);
		void watchServer();
		void fileParser(char *av);
		void print_map();
		std::string trimSpaces(std::string line);
		void print_location_map();
		int scopeValidation(std::ifstream &file);
		int inputData(std::string line);
		int inputLocation(std::string line, t_location &location);
		std::string trimSemiColon(std::string val);
		int validateHTTPCode(std::string &val);
	
};

// Template function to print any map
template <typename K, typename V>
void printMap(const std::map<K, V> &m) {
    for (typename std::map<K, V>::const_iterator it = m.begin(); it != m.end(); ++it) {
        std::cout << it->first << " -> " << it->second << std::endl;
    }
}
class ConfigFileError : public std::runtime_error {
public:
	ConfigFileError(); // constructor declaration
};

#endif