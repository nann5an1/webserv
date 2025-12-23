#include "Webserv.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Utils.hpp"
#include "Cgi.hpp"

#include <pwd.h>

int main(int ac, char **av)
{
	int	status = 0;
	if(ac > 2) 
		return (1);
	init_global();
	try
	{
		Webserv webserv;

		av[1] = (char *)(std::string(getpwuid(getuid())->pw_name) + ".conf").c_str();
        
		webserv.fileParser(av[1]);
		// webserv.printServers();

		std::cout << "========== " << av[1] << " ==========" << std::endl;
		

		// const char* raw_request = 
		// "POST /upload HTTP/1.1\r\n"
		// "Host: localhost:8080\r\n"
		// "Transfer-Encoding: chunked\r\n"
		// "Content-Type: application/x-www-form-urlencoded\r\n"
		// "\r\n"
		// "8\r\n"            // chunk size in hex (7 bytes)
		// "username\r\n"     // chunk data
		// "3\r\n"            // next chunk (3 bytes)
		// "=ab\r\n"
		// "2\r\n"            // next chunk (2 bytes)
		// "&x\r\n"
		// "2\r\n"            // next chunk (1 byte)
		// "=1\r\n"
		// "0\r\n"            // last chunk, 0 size
		// "\r\n";            // end of chunks


		 const char* raw_request = 
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Content-Type: multipart/form-data; boundary=----XYZ789\r\n"
        "Content-Length: 407\r\n"
        "\r\n"
        "------XYZ789\r\n"
        "Content-Disposition: form-data; name=\"file1\"; filename=\"test.jpg\"\r\n"
        "Content-Type: image/jpeg\r\n"
        "\r\n"
        "FAKEBINARYDATA1FAKEBINARYDATA1FAKEBINARYDATA1\r\n"
        "------XYZ789\r\n"
        "Content-Disposition: form-data; name=\"file2\"; filename=\"example.bin\"\r\n"
        "Content-Type: application/octet-stream\r\n"
        "\r\n"
        "FAKEBINARYDATA2FAKEBINARYDATA2FAKEBINARYDATA2\r\n"
        "------XYZ789\r\n"
        "Content-Disposition: form-data; name=\"file3\"; filename=\"sample.txt\"\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "Hello world from sample.txt\r\n"
        "------XYZ789--\r\n";
		
		Request req;
		req.parseRequest(raw_request);
		// testing_request();
		// if (webserv.start())
		// 	return (1);
		

	} catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	
    return (0);
}

void	testing_request()
{
	// const char* raw_req = 
    // "POST /script.php HTTP/1.1\r\n"
    // "Host: localhost:8080\r\n"
    // "Content-Type: application/x-www-form-urlencoded\r\n"
    // "Content-Length: 19\r\n";

	const char* raw_req = 
	"POST /upload HTTP/1.1\r\n"
	"Host: localhost:8080\r\n"
	"Transfer-Encoding: chunked\r\n"
	"Content-Type: application/x-www-form-urlencoded\r\n"
	"\r\n"
	"13\r\n"
	"username=abc&x=1\r\n"
	"0\r\n"
	"\r\n";

	Request req;
	req.parseRequest(raw_req);

}

void	testing_path(Webserv &webserv)
{
	const char* tempPaths[] = {"/autoindex/", "/autoindex", "/autoindex/smth", "/smth", "/"};
	std::vector<std::string> paths(tempPaths, tempPaths + 5);
	std::string loc;

	const t_location	*location = NULL;
	Server	server = webserv._servers[0];
	for (int j = 0; j < paths.size(); ++j)
	{
		std::string path = paths[j], final = "", remain = "";
		for (int i = path.size(); i >= 0; --i)
		{
			if (path[i] == '/' || i == path.size())
			{
				loc = path.substr(0, i);
				if (i == 0)
					loc = "/";
				location = get(webserv._servers[0].locations(), loc);
				if (location)
				{

					std::string	root = location->root.empty() ? server.root() : location->root;
					remain = path.substr(i);
					if (loc == "/")
						loc = "";
					final = root + loc + remain;
					std::cout << j << " final: " << final << std::endl;

					break ;
				}

			}
		}
	}
}