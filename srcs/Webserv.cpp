#include "Webserv.hpp"
#include "Server.hpp"
#include "Response.hpp"

Webserv::Webserv() {
	
}

Webserv::~Webserv()
{
	if (_ep_fd >= 0)
		close(_ep_fd);
}

Webserv::Webserv(const Webserv &other) {
	(void)other;
}

Webserv& Webserv:: operator=(const Webserv &other) {
	(void)other;
	return *this;
}

//check if the {} is even for the server and for the location scopes
//only the validation of the scopes should happen here no value parsing
int Webserv::scopeValidation(std::ifstream &file)
{
	std::string line, tok1, tok2;
	bool location_scope = false;
	// bool server_scope = false;
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

void Webserv::fileParser(char *av)
{
	std::ifstream inputFile;
	inputFile.open(av);
	std::string	word;
	std::string	config_file;

	if(av) config_file = av;
	else config_file = "def.conf";

	// std::cout << "config file >> "<< config_file << std::endl;
	std::ifstream	file(config_file.c_str());
	std::string line;

	// if(scopeValidation(file) == 0) throw ConfigFileError();
	// 	file.clear();
	// 	file.seekg(0);
		
	while(getline(file, line))
	{
		std::stringstream	ss(line);
		std::string			tok;
		while (ss >> tok)
		{
			if (tok == "server")
				_servers.push_back(Server(file));
		}
	}
}

//print each of the servers frin the webserv
void Webserv::printServers() const {
    std::cout << "\n========== PRINTING ALL SERVERS ==========\n" << _servers.size() << std::endl;
    for (size_t i = 0; i < _servers.size(); i++) {
        std::cout << "\nSERVER #" << i + 1 << ":\n";
        _servers[i].print();
    }
}

ConfigValidationError::ConfigValidationError()
	: std::runtime_error("Error in config file") {}

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

int	Webserv::servers_start(std::set<fd>& server_fds)
{
	int status = 0;
	int	size = _servers.size();
	std::cerr << "[webserv]\tstarting " << size << " server(s)" << std::endl;
	for (int i = 0; i < size; ++i)
	{
		Server	&server = _servers[i];
		if (server.start())
			continue ;
		_servers_map[server] = &server;
		server_fds.insert(server);
	}
	if (!server_fds.size())
		return (-1);
	return (0);
}

int	Webserv::server_add()
{
	for(std::size_t i = 0; i < _servers.size(); ++i)
	{
		fd	s_fd = _servers[i];
		if (s_fd < 0)
			continue;
		struct epoll_event	s_event;
		s_event.events = EPOLLIN;
		s_event.data.fd = s_fd;
		// register server socket with epoll
		if (epoll_ctl(_ep_fd, EPOLL_CTL_ADD, s_fd, &s_event) <  0)
			return (fail("Epoll: Server", errno));
	}
	return (0);
}

int	Webserv::create_con(const Server* server)
{
	while (true)
	{
		Connection	con(server);
		fd c_fd = con;
		if (c_fd < 0)
			break;

		struct epoll_event	c_event;
		c_event.events = EPOLLIN | EPOLLET;
		c_event.data.fd = c_fd;

		// FAIL~
		if (epoll_ctl(_ep_fd, EPOLL_CTL_ADD, c_fd, &c_event) < 0)
		{
			int status = fail("Epoll: Client", errno); 
			close(c_fd);
			return (status);
		}
		_cons[c_fd] = con;
	}
	return (0);
}

int	Webserv::start()
{
	int	status = 0;

	_ep_fd = epoll_create(1);
	std::set<fd>	server_fds;
	if (_ep_fd < 0)
		return (fail("Epoll", errno));
	if (servers_start(server_fds) < 0)
		return (fail("Servers: No servers were started successfully!", -1));
	if ((status = server_add()))
		return (status);
	std::cout << "[webserv]\tservers registering succeed" << std::endl;

	epoll_event	events[MAX_EVENTS];

 	while (true)
	{
		int	hits = epoll_wait(_ep_fd, events, MAX_EVENTS, 1000);
		if (hits < 0)
		{
			if (errno == EINTR)
				continue ;
			fail("Epoll", errno);
			break;
		}
		for (int i = 0; i < hits; ++i)
		{
			fd	event_fd = events[i].data.fd;
			if (server_fds.count(event_fd))
			{
				if (create_con(_servers_map[event_fd]))
				{
					fail("Connection", errno);
					continue;
				}
			}
			else
			{
				std::map<fd, Connection>::iterator it = _cons.find(event_fd);
				if (it == _cons.end())
					continue;
				Connection	&cur_con = it->second;
				if (events[i].events & EPOLLIN)
				{
					if (cur_con.request())
					{
						struct	epoll_event	mod_event;
						mod_event.events = EPOLLOUT | EPOLLET;
						mod_event.data.fd = event_fd;
						epoll_ctl(_ep_fd, EPOLL_CTL_MOD, event_fd, &mod_event);
					}
					else
						con_close(event_fd);
				}
				else if (events[i].events & EPOLLOUT)
				{
					if (cur_con.response())
						con_close(event_fd);
				}
				else if (events[i].events & (EPOLLHUP | EPOLLERR))
				{
					std::cout << "epollhup epollerr" << std::endl;
					con_close(event_fd);
				}
			}
		}
		// timeout();
	}
	for (std::set<fd>::iterator it = server_fds.begin(); it != server_fds.end(); ++it)
		close(*it);
	close(_ep_fd);
	return (0);
}

void	Webserv::timeout()
{
	time_t	now = time(NULL);
	for (std::map<fd, Connection>::iterator it = _cons.begin(); it != _cons.end();)
	{
		int	c_fd = it->first;
		if (now - it->second > WAIT_TIME)
		{
			std::cout << "[webserv]\tclient timeout\t\t\t| socket:" << c_fd << std::endl;
			std::map<fd, Connection>::iterator	tmp = it++;
			con_close(c_fd);
		}
		else
			++it;
	}
}

void	Webserv::con_close(fd fd_)
{
	epoll_ctl(_ep_fd, EPOLL_CTL_DEL, fd_, NULL);
	std::cout << "[webserv]\tclient disconnected\t\t| socket:" << fd_ << std::endl;
	_cons.erase(fd_);
	close(fd_);
}

//trim spaces/tabs and validate
// std::string Webserv::trimSpaces(std::string line){
// 	std::string serverHeadline = "";
	
// 	// std::cout << line.length() << std::endl;

// 	for(size_t i = 0; i < line.length(); i++){
// 		if(serverHeadline != "server" && line[i] == ' ' && serverHeadline.length() != 0) break;
// 		if(serverHeadline.length() == 0 && (line[i] == ' ' || line[i] == '\t')) continue;
// 		if(line[i] != ' ')
// 			serverHeadline += line[i];
// 	}
// 	// std::cout << "after spaces trimmed >> " << serverHeadline << std::endl;
// 	if(serverHeadline.length() == 7 && serverHeadline == "server{") return serverHeadline;
// 	else if(serverHeadline.length() == 6 && serverHeadline == "server") return serverHeadline;
// 	else if(serverHeadline.length() == 1 && serverHeadline == "{") return serverHeadline;
// 	else return "";
		
// 	return (serverHeadline);
// }

// Note: `start()` is complex and uses edge-triggered epoll; review carefully before changing.