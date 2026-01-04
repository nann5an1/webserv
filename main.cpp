#include "Webserv.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Utils.hpp"


int main(int ac, char **av)
{
	if(ac > 2) 
		return (1);
	init_global();
	try
	{
		Webserv webserv;

		webserv.fileParser(av[1]);
		if (webserv.start())
			return (1);
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	
    return (0);
}

