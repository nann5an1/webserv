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

        std::cout << "========== " << av[1] << " ==========" << std::endl;
		webserv.fileParser((char *)"aoo.conf");
		webserv.printServers();
		// if (webserv.start())
		// 	return (1);
	} catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	
    return (0);
}