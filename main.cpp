#include "Webserv.hpp"
#include "Server.hpp"

int main(int ac, char **av)
{
    if(ac > 2) return 1;
    
    Webserv webserv;
    webserv.fileParser(av[1]);
    // webserv.watchServer();

    Server	server;
	int		status;
	// if ((status = server.start()) != 0)
	// 	std::cerr << RED << "Error: Socket: " << strerror(status) << RESET << std::endl;

	fd	sock = server;
	std::cout << "port: " << server << std::endl;
	std::cout << "sock: " << sock << std::endl;
	std::cout << "id: " << (std::string)server << std::endl;
}