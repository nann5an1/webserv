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
	try
	{
    Webserv webserv;
	#if defined(__x86_64__)
		av[1] = (char *)"aoo.conf";
	#endif
	webserv.fileParser(av[1]);
	webserv.printServers();
	if (webserv.start())
		return (1);
	} catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	
    return (0);
}