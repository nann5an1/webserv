#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <vector>
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
#include <map>
#include <exception>
#include "Server.hpp"

class Webserv{
	private:
		std::vector<Server>servers;	//take the sever class as type and save a collection of servers
	public:
		Webserv();
		Webserv(char *av); //take the av(filename) like this
		~Webserv();
		Webserv(const Webserv &other);
		Webserv& operator=(const Webserv &other);
		void watchServer();
		void fileParser(char *av);
		void print_map();
	
		void print_location_map();

		int scopeValidation(std::ifstream &file);

		
		// int inputData(std::string line);
		// int inputLocation(std::string line, t_location &location);
		// std::string trimSemiColon(std::string val);
		// int validateHTTPCode(std::string &val);
	
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