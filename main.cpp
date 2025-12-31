#include "Webserv.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Utils.hpp"
#include "Cgi.hpp"

#include <pwd.h>

void	testing_request()
{
	const char* raw_reqs[] = {

    // 1. Simple text body */ //normal chunking
    "POST /echo HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Transfer-Encoding: chunked\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "5\r\n"
    "Hello\r\n"
    "6\r\n"
    " World\r\n"
    "0\r\n"
    "\r\n",

    /* 2. Chunk split logically */
    "POST /data HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Transfer-Encoding: chunked\r\n"
    "\r\n"
    "A\r\n"
    "01234"
    "56789\r\n"
    "0\r\n"
    "\r\n",

   /* 3. JSON request (CGI) - CORRECTED */
    "POST /cgi-bin/api HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Transfer-Encoding: chunked\r\n"
    "Content-Type: application/json\r\n"
    "\r\n"
    "6\r\n"        // 6 bytes, not 7
    "{\"id\":"      // no \r\n here!
    "\r\n"         // chunk terminator
    "8\r\n"
    " 42,\"ok\""
    "\r\n"
    "6\r\n"        // 6 bytes, not 7
    ":true}"       // no \r\n here!
    "\r\n"
    "0\r\n"
    "\r\n",

    // /* 4. Multipart file upload */
    "POST /upload HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Transfer-Encoding: chunked\r\n"
    "Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryX\r\n"
    "\r\n"
    "1A\r\n"
    "------WebKitFormBoundaryX\r\n"
    "Content-Disposition: form-data; name=\"file\"; filename=\"a.txt\"\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "Hello\r\n"
    "1A\r\n"
    "World\r\n"
    "------WebKitFormBoundaryX--\r\n"
    "0\r\n"
    "\r\n",

    /* 5. Chunk extensions */
    "POST /ext HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Transfer-Encoding: chunked\r\n"
    "\r\n"
    "5;foo=bar\r\n"
    "Hello\r\n"
    "0\r\n"
    "\r\n",


	
	/* 7. Normal Upload */
	"POST /upload HTTP/1.1\r\n"
	"Host: example.com\r\n"
	"Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryX\r\n"
	"Content-Length: 165\r\n"
	"\r\n"
	"------WebKitFormBoundaryX\r\n"
	"Content-Disposition: form-data; name=\"file\"; filename=\"a.txt\"\r\n"
	"Content-Type: text/plain\r\n"
	"\r\n"
	"Hello\n"
	"World\n"
	"------WebKitFormBoundaryX--\r\n",

    NULL
	};

	
	// for (size_t i = 0; raw_reqs[i] != NULL; ++i)
	// {
	// 	const char* req_str = raw_reqs[i];
	// 	Request req;
	// 	req.parseRequest(req_str);
	// 	// std::cout << "body: " << req.body() << std::endl;
	// 	req.printUploadedFiles();
	// 	// std::cout << "\n" << std::endl;
	// 	/* feed req into your parser */
	// }
}

int main(int ac, char **av)
{
	int	status = 0;
	if(ac > 2) 
		return (1);
	init_global();
	try
	{
		Webserv webserv;

		// av[1] = (char *)(std::string(getpwuid(getuid())->pw_name) + ".conf").c_str();

		webserv.fileParser(av[1]);
		// webserv.printServers();

		// std::cout << "========== " << av[1] << " ==========" << std::endl;
		
		// testing_request();
		if (webserv.start())
			return (1);
		

	} catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	
    return (0);
}

// void	testing_path(Webserv &webserv)
// {
// 	const char* tempPaths[] = {"/autoindex/", "/autoindex", "/autoindex/smth", "/smth", "/"};
// 	std::vector<std::string> paths(tempPaths, tempPaths + 5);
// 	std::string loc;

// 	const t_location	*location = NULL;
// 	Server	server = webserv._servers[0];
// 	for (int j = 0; j < paths.size(); ++j)
// 	{
// 		std::string path = paths[j], final = "", remain = "";
// 		for (int i = path.size(); i >= 0; --i)
// 		{
// 			if (path[i] == '/' || i == path.size())
// 			{
// 				loc = path.substr(0, i);
// 				if (i == 0)
// 					loc = "/";
// 				location = get(webserv._servers[0].locations(), loc);
// 				if (location)
// 				{

// 					std::string	root = location->root.empty() ? server.root() : location->root;
// 					remain = path.substr(i);
// 					if (loc == "/")
// 						loc = "";
// 					final = root + loc + remain;
// 					std::cout << j << " final: " << final << std::endl;

// 					break ;
// 				}

// 			}
// 		}
// 	}
// }