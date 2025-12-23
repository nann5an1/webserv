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
		webserv.printServers();
		std::cout << "========== " << av[1] << " ==========" << std::endl;
		// if (webserv.start())
		// 	return (1);
		

	} catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	
    return (0);
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