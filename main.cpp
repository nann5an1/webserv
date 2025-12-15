#include "Webserv.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Utils.hpp"
#include "Cgi.hpp"

int main(int ac, char **av)
{
	int	status = 0;
	if(ac > 2) 
		return (1);
	init_global();
    Webserv webserv;
	#if defined(__x86_64__)
		av[1] = (char *)"aoo.conf";
	#endif
	webserv.fileParser(av[1]);
	webserv.printServers();

	if (webserv.start())
		return (1);
    return (0);
}