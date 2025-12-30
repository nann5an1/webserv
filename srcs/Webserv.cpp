#include "Webserv.hpp"

Webserv::Webserv() {}

Webserv::Webserv(const Webserv &other) : _servers(other._servers) {}

Webserv	&Webserv::operator=(const Webserv &other)
{
	if (this != &other)
		_servers = other._servers;
	return(*this);
}

Webserv::~Webserv() {}

bool	Webserv::servers_start()
{
	int status = 0;
	int	size = _servers.size(), fail_count = 0;
	std::cerr << "[webserv]\tstarting " << size << " server(s)" << std::endl;
	for (int i = 0; i < size; ++i)
	{
		Server	&server = _servers[i];
		if (server.start())
		{
			fail_count++;
			continue ;
		}
	}
	return (fail_count == size);
}

bool	Webserv::server_add()
{
	int	fail_count = 0;
	for(std::size_t i = 0; i < _servers.size(); ++i)
	{
		Server	&server = _servers[i];
		fd		s_fd = server;
		if (s_fd < 0)
		{
			fail ("Epoll: Server: " + server.name() + " failed", -1);
			fail_count++;
			continue;
		}
		if (Epoll::instance().add_fd(&server, s_fd, EPOLLIN | EPOLLET) < 0)
			return (fail("Epoll: Server", errno));
	}
	return (fail_count == _servers.size());
}

void	Webserv::fileParser(char *av)
{
	std::string		config_file, line;
	
	config_file = av ? av : "def.conf";
	
	std::ifstream	file(config_file.c_str());
		
	while(getline(file, line))
	{
		std::stringstream	ss(line);
		std::string			tok = "";
		while (ss >> tok)
		{
			if (tok == "server")
				_servers.push_back(Server(file));
		}
	}
}

int	Webserv::scopeValidation(std::ifstream &file)
{
	std::string line, tok1, tok2;
	bool location_scope = false;
	t_location location;

	int start_location = 0, end_location = 0;
	int start_server = 0, end_server = 0;

	/*------------------------inside the server scope------------------*/

	//if we are in the server scope now read the line until the ending of the server's }
	while(getline(file, line)){
		// std::cout << "line check >> " << line << std::endl;
		std::string token;
		std::stringstream ss(line);
		ss >> token;
		if(token == "server"){
			std::stringstream ss(line);

			ss >> tok1 >> tok2;
			// std::cout << "line " << line << std::endl;
			// std::cout << "token " << token << std::endl;
			// std::cout << "tok2 " << tok2 << std::endl;

			//handle the { variations
			if(tok2 == " "){
				std::string tok;
				getline(file, line);
				std::stringstream ss(line);
				ss >> tok;
				if(tok != "{")	return 0;
			}
			// server_scope = true;
			start_server++;
		}
		else if(line.find("location") != std::string::npos){
			std::stringstream ss(line);
			std::string tok1, tok2, tok3;
			ss >> tok1 >> tok2 >> tok3;

			// std::cout << "token1 location >> " << tok1 << std::endl;
			// std::cout << "token2 location >> " << tok2 << std::endl;
			// std::cout << "token3 location >> " << tok3 << std::endl;
			
			if(tok1 != "location") return 0;
			if(tok3 != "{"){ //if the opening isn't on the same line	
				getline(file, line); //read the next line if the opening isn't on the same line
				// std::cout << "Moved to another line of location >> " << line << std::endl;
				std::stringstream ss(line);
				ss >> tok1;
				if(tok1 != "{") return 0;
				else{
					location_scope = true;
					start_location++;
				}
			}
			else{
				location_scope = true;
				start_location++;
			}
		}
		if(location_scope && line.find("}") != std::string::npos){ //after reading the location scope
			location_scope = false;
			//need to add the values of the whole location scope into the map
			end_location++;
		}
		else if(location_scope == false && line.find("}") != std::string::npos){ //look for the end of the server scope
			std::stringstream ss(line);
			std::string tok1;
			ss >> tok1;

			// std::cout << "token of ending server scope >> " << tok1 << std::endl;
			if(tok1 == "}") end_server++;
		}
	}
	// std::cout << "start server_count " << start_server << std::endl;
	// std::cout << "end server_count " << end_server << std::endl;

	// std::cout << "start location_count " << start_location << std::endl;
	// std::cout << "end location_count " << end_location << std::endl;

	
	//return 0 if the number of server and location scopes are not even
	if(start_server != end_server || start_location != end_location) return 0;
	return 1;
}

void	Webserv::print_server_head() const
{
	std::cout << "server count : " << _servers.size() << std::endl;
	for (int i = 0; i < _servers.size(); ++i)
	{
		const Server	&server = _servers[i];
		fd	s_fd = server;
		std::cout << std::string(server) << "\t: " << s_fd << std::endl;
	}
}

void	Webserv::printServers() const
{
	std::cout << "\n========== PRINTING ALL SERVERS ==========\n" << _servers.size() << std::endl;
	for (size_t i = 0; i < _servers.size(); i++)
	{
		std::cout << "\nSERVER #" << i + 1 << ":\n";
		_servers[i].print();
	}
}

int	Webserv::start()
{
	int	status = 0;

	if (Epoll::instance().init() < 0)
	{
		fail("Epoll", errno);
		throw Error("Webserv: Epoll: failed to create");
	}
	if (servers_start())
	{
		fail("Servers: No servers were started successfully!", -1);
		throw Error("Webserv: Server: failed");
	}
	if (server_add())
	{
		fail("Servers: No servers were added to epoll!", -1);
		throw Error("Webserv: Epoll: server adding failed");
	}
	std::cout << "[webserv]\tservers registering succeed" << std::endl;

	epoll_event	events[MAX_EVENTS];

 	while (true)
	{
		int	hits = Epoll::instance().wait(events, MAX_EVENTS, 1000);
		if (hits < 0)
		{
			if (errno == EINTR)
				continue ;
			fail("Epoll", errno);
			break;
		}
		for (int i = 0; i < hits; ++i)
		{
			IPollable	*poll_obj = static_cast<IPollable*>(events[i].data.ptr);
			if (poll_obj)
			{
				poll_obj->handle(events[i].events);
				continue;
			}
		}
		// timeout();
	}
	return (0);
}

