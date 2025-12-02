#include "Webserv.hpp"
#include "Server.hpp"
#include "Request.hpp"


int main(int ac, char **av)
{
    if(ac > 2) return 1;
    
    Webserv webserv;
    webserv.fileParser(av[1]);
	webserv.printServers();
    // webserv.watchServer();

	// const char* raw_request = 
    //     "POST /upload HTTP/1.1\n"
    //     "Host: localhost:8080\n"
    //     "Content-Type: application/json\n" 
    //     "Content-Length: 27\n"                     
    //     "Connection: keep-alive\n";

    // const char* raw_request2 =
    //     "GET /script.pl?name=john&age=20 HTTP/1.1\n"
    //     "Host: localhost:8080\n"
    //     "Accept: text/html\n"
    //     "Connection: close";

    const char* raw_request3 = 
    "POST /script.php HTTP/1.1\n"
    "Host: localhost:8080\n"
    "Content-Type: application/x-www-form-urlencoded\n"
    "Content-Length: 19\n"

    "name=john&age=20";

	Request req;
    req.parseRequest(raw_request3);

    // if (req.parseRequest(raw_request)) {
    //     std::cout << "Method: " << req.getMethod() << "\n";
    //     std::cout << "Path: " << req.getPath() << "\n";
    //     std::cout << "Query: " << req.getQuery() << "\n";
    //     std::cout << "HTTP Version: " << req.getHttpVersion() << "\n";
    //     std::cout << "Host: " << req.getHostname() << "\n";
    //     std::cout << "Port: " << req.getPort() << "\n";
    //     std::cout << "Connection: " << req.getConnStatus() << "\n";
    // }
    return 0;
    // Server	server;
	// int		status;
	// if ((status = server.start()) != 0)
	// 	std::cerr << RED << "Error: Socket: " << strerror(status) << RESET << std::endl;

	// fd	sock = server;
	// std::cout << "port: " << server << std::endl;
	// std::cout << "sock: " << sock << std::endl;
	// std::cout << "id: " << (std::string)server << std::endl;
}