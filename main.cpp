#include "Webserv.hpp"
#include "Server.hpp"
#include "Request.hpp"

extern std::map<int, const char*>	gphrase;

extern const std::string	CRLF = "\r\n";

void	init_gphrase()
{
	// 0xx Not found
	gphrase[0] = "";

	// 1xx Informational
	gphrase[100] = "Continue";
	gphrase[101] = "Switching Protocol";
	gphrase[102] = "Processing";

    // 2xx Success
	gphrase[200] = "OK";
	gphrase[201] = "Created";
	gphrase[202] = "Accepted";
	gphrase[204] = "No Content";
	gphrase[206] = "Partial Content";

	// 3xx Redirection
	gphrase[300] = "Multiple Choices";
	gphrase[301] = "Moved Permanently";
	gphrase[302] = "Found";
	gphrase[303] = "See Other";
	gphrase[304] = "Not Modified";
	gphrase[307] = "Temporary Redirect";
	gphrase[308] = "Permanent Redirect";

	// 4xx Client errors
	gphrase[400] = "Bad Request";
	gphrase[401] = "Unauthorized";
	gphrase[403] = "Forbidden";
	gphrase[404] = "Not Found";
	gphrase[405] = "Method Not Allowed";
	gphrase[408] = "Request Timeout";
	gphrase[409] = "Conflict";
	gphrase[411] = "Length Required";
	gphrase[413] = "Payload Too Large";
	gphrase[414] = "URI Too Long";
	gphrase[415] = "Unsupported Media Type";
	gphrase[418] = "I'm a teapot"; // optional
	gphrase[429] = "Too Many Requests";

	// 5xx Server errors
	gphrase[500] = "Internal Server Error";
	gphrase[501] = "Not Implemented";
	gphrase[502] = "Bad Gateway";
	gphrase[503] = "Service Unavailable";
	gphrase[504] = "Gateway Timeout";
	gphrase[505] = "HTTP Version Not Supported";
}

int main(int ac, char **av)
{
    if(ac > 2) return 1;

	init_gphrase();
    Webserv webserv;
    webserv.fileParser(av[1]);
    webserv.start();

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