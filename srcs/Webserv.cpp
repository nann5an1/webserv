#include "Webserv.hpp"
#include "Server.hpp"

Webserv::Webserv(){
}

Webserv::~Webserv(){}

Webserv::Webserv(const Webserv &other) {
	(void)other;
}

Webserv& Webserv:: operator=(const Webserv &other) {
	(void)other;
	return *this;
}

void Webserv::watchServer(){
	// while(epoll())
	// int epoll_fd = epoll_create(10);
	// std::cout << epoll_fd << std::endl;

	//if successful returns 0
	// epoll_ctl(epoll_fd, EPOLL_CTL_ADD, )
}



// int Webserv::inputLocation(std::string line, t_location &location){
// 	std::string token;
	
// 	std::stringstream ss(line);
// 	ss >> token;

// 	if(token == "autoindex"){
// 		std::string val;
// 		ss >> val;
// 		val = trimSemiColon(val);
// 		if(val == "on") location.autoindex = true;
// 	}
// 	else if(token == "allow_methods"){
// 		std::string method1, method2, method3;
// 		ss >> method1 >> method2 >> method3;
// 		// std::cout << "methods >> " << method1 << " " << method2 << " " << method3 << std::endl;

// 		if(method1.find(";") != std::string::npos)	method1 = trimSemiColon(method1);
// 		if(method2.find(";") != std::string::npos)	method2 = trimSemiColon(method2);
// 		if(method3.find(";") != std::string::npos)	method3 = trimSemiColon(method3);

// 		if(method1 == "GET")	location.get = true;
// 		if(method2 == "POST")	location.post = true;
// 		if(method3 == "DELETE")	location.del = true;
// 	}
// 	else if(token == "root"){
// 		std::string val;
// 		ss >> val;
// 		val = trimSemiColon(val);
// 		location.root = val;
// 	}
// 	else if(token == "upload_dir"){
// 		std::string val;
// 		ss >> val;
// 		val = trimSemiColon(val);
// 		location.upload_dir = val;
// 	}
// 	else if(token == "index"){
// 		std::string val1, val2;
// 		ss >> val1 >> val2;
// 		location.index_files.push_back(val1);
// 		location.index_files.push_back(val2);
// 		// std::cout << location.index_files[0] << " " << location.index_files[1] << std::endl;
// 	}
// 	else if(token == "cgi"){
// 		std::string key, val;
// 		ss >> key >> val;
// 		val = trimSemiColon(val);
// 		location.cgi.insert(std::pair<std::string, std::string>(key, val));
// 	}
// 	else if(token == "return"){
// 		std::string key, val;
// 		ss >> key >> val;
// 		val = trimSemiColon(val);
// 		if(!validateHTTPCode(key)) return 0;
// 		location.ret_pages.insert(std::pair<int, std::string>(atoi(key.c_str()), val));
// 	}
// 	return 1;
// }


//check if the {} is even for the server and for the location scopes
//only the validation of the scopes should happen here no value parsing
int Webserv::scopeValidation(std::ifstream &file){
	std::string line, tok1, tok2;
	bool location_scope = false;
	bool server_scope = false;
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
			server_scope = true;
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
		// if(server_scope && location_scope == false){	//inputting the data from the server scope
		// 	if(!inputData(line)) return 0;
			
		// }
		// else if(server_scope && location_scope){	//inputting the data from the location scope
		// 	if(!inputLocation(line, location)) return 0;

			// std::cout << "get: " << location.get << std::endl;
			// std::cout << "post: " << location.post << std::endl;
			// std::cout << "del: " << location.del << std::endl;

		// }
	}
	// std::cout << "start server_count " << start_server << std::endl;
	// std::cout << "end server_count " << end_server << std::endl;

	// std::cout << "start location_count " << start_location << std::endl;
	// std::cout << "end location_count " << end_location << std::endl;

	
	//return 0 if the number of server and location scopes are not even
	if(start_server != end_server || start_location != end_location) return 0;
	return 1;
}

void Webserv::fileParser(char *av){
	//when the server constructor has been successfully created, the server's whole scope from the config
	//file will be passed into the _servers
	std::ifstream inputFile;
	inputFile.open(av);

	//validation first for the whole config file
	//get the pointer and while moving, parse the file into the server's attributes
	//then push that server back to the collection of _servers in the webserv

	/* //this is not validation, this is parsing!!!
	while(reading until the end of the config file)
		if(server keyword found){
			- Server(pass reference of the file from the ifstream) -> this will create the object
			- from under the object, can do the value parsing
				-read line by line until the end of the server scope }
					-quit the reading when the server scope ends
		}
	-- since we r using ifstream, when we quit as we found the }, it will just automatically update
	the location of that place
	*/

	
	std::string	word;
	std::string	config_file;
	if(av) config_file = av;
	else config_file = "def.conf";

	std::ifstream file(config_file.c_str());
	std::string line;

	if(scopeValidation(file)){ //if scope validation works, move to the parsing for each of the server scope
		// std::cout << "valid config file" << std::endl;
		// std::cout << "/nserver_name: " << this->server_name << std::endl;
		// std::cout << "listen_ip: " << this->listen_ip << std::endl;
		// std::cout << "listen_port: " << this->listen_port << std::endl;
		// std::cout << "max_body_size: " << this->max_body_size << std::endl;
		
		// _servers.pushback(Server(file)); //cannot be called inside the constructor
		while(getline(file, line)){ //read the whole file line by line
			if(line.find("server") != std::string::npos){
				std::string tok1, tok2;
				std::stringstream ss(line);
				int server_scope = 1;
				
				while(ss >> tok1 >> tok2){
					// if(tok2 == "{")	server_scope++;
					if(tok1 == "server") {
						server_scope++;
						Server(file, server_scope);
						
					}
					
				}
			}
		}
		// Server(file);
		// printMap(this->err_pages);
		
		// printMap(this->location_map);
	}
	else{
		throw ConfigFileError();
	}
	// std::cout << "server_scope status -> " << server_scope << std::endl;
}


//print location map
// void Webserv::print_location_map(){
// 	for(std::map<std::string, t_location>::iterator it = this->location_map.begin(); it != this->location_map.end(); ++it){
// 		std::cout << " " << it->first << " " 
// 		<< it->second.autoindex << " " 
// 		<< it->second.get << " " 
// 		<< it->second.post << " " 
// 		<< it->second.del << " " 
// 		<< it->second.upload_dir << " "
// 		<< it->second.root << std::endl;
// 	}
// }




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

//config file parser
// Webserv::fileParse(char *av){
	
// }

int	Webserv::fail(std::string head, int err_no)
{
	std::cerr << RED << "Error: " << head << ": " << strerror(err_no) << std::endl;
	return (err_no);
}


int	Webserv::start()
{
	fd	ep_fd = epoll_create(1);
	std::set<fd>	server_fds;
	if (ep_fd < 0)
		return (errno);
	for(std::size_t i = 0; i < _servers.size(); ++i)
	{
		fd	s_fd = _servers[i];
		struct epoll_event	s_event;
		s_event.events = EPOLLIN;
		s_event.data.fd = s_fd;
		epoll_ctl(ep_fd, EPOLL_CTL_ADD, s_fd, &s_event);
		server_fds.insert(s_fd);
	}

	epoll_event	events[MAX_EVENTS];
	
	std::map<fd, std::time_t>	timestamps;

	while (true)
	{
		int	hits = epoll_wait(ep_fd, events, MAX_EVENTS, WAIT_TIME);
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
				while (true)
				{
					sockaddr_in	client_addr;
					socklen_t	client_len = sizeof(client_addr);
					fd	c_fd = accept(event_fd, (sockaddr *)&client_addr, &client_len);
					if (c_fd < 0)
					{
						if (errno == EAGAIN || errno == EWOULDBLOCK)
							break;
						fail("Epoll", errno);
						break;
					}
					// FAIL~ need proper clean up
					if (fcntl(c_fd, F_SETFL, fcntl(c_fd, F_GETFL, 0) | O_NONBLOCK) < 0)
						return (errno);

					struct epoll_event	c_event;
					c_event.events = EPOLLIN | EPOLLET;
					c_event.data.fd = c_fd;

					// FAIL~
					if (epoll_ctl(ep_fd, EPOLL_CTL_ADD, c_fd, &c_event) < 0)
					{
						int	status = errno; 
						close(c_fd);
						return (status);
					}
					timestamps[c_fd] = time(NULL);
				}
			}
			else if (events[i].events & EPOLLIN)
			{
				//	HELP: No understanding at all
				//	HELP: I dont understand a shit at all starting from here.
				char	buffer[4096];
				bool	keep = true;
				while (true)
				{
					ssize_t	bytes = read(event_fd, buffer, sizeof(buffer));
					if (bytes > 0)
					{
						std::string	req(buffer, bytes);
						std::cout << "Request: " << event_fd << "\n" << std::string(42, '=') << "\n" << req << std::endl;
						timestamps[event_fd] = time(NULL);
					}
					else if (bytes == 0)
					{
						keep = false;
						break;
					}
					else
					{
						if (errno == EAGAIN)
							break;
						keep = false;
						break;
					}
				}
				if (!keep)
				{
					epoll_ctl(ep_fd, EPOLL_CTL_DEL, event_fd, NULL);
					close(event_fd);
					timestamps.erase(event_fd);
				}
				else
				{
					struct	epoll_event	mod_event;
					mod_event.events = EPOLLOUT | EPOLLET;
					mod_event.data.fd = event_fd;
					epoll_ctl(ep_fd, EPOLL_CTL_MOD, event_fd, &mod_event);
				}
			}
			else if (events[i].events & EPOLLOUT)
			{
				const char response[] = "HTTP/1.1 200 OK\r\n"
										"Content-Length: 12\r\n"
										"Content-Type: text/plain\r\n"
										"\r\n"
										"Hello World";
				;
				if (write(event_fd, response, sizeof(response) - 1) < 0 && errno != EAGAIN)
					fail("Response", errno);
				epoll_ctl(ep_fd, EPOLL_CTL_DEL, event_fd, NULL);
				close(event_fd);
				timestamps.erase(event_fd);
			}
			else if (events[i].events & (EPOLLHUP | EPOLLERR))
			{
				epoll_ctl(ep_fd, EPOLL_CTL_DEL, event_fd, NULL);
				close(event_fd);
				timestamps.erase(event_fd);
			}
		}

		time_t	now = time(NULL);
		for (std::map<fd, time_t>::iterator it = timestamps.begin(); it != timestamps.end();)
		{
			int	c_fd_ = it->first;
			if (now - it->second > WAIT_TIME)
			{
				std::cout << "Client " << c_fd_ << "time out" << std::endl;
				epoll_ctl(ep_fd, EPOLL_CTL_DEL, c_fd_, NULL);
				close(c_fd_);
				it = timestamps.erase(it);
			}
			else
				++it;
		}
	}
	for (std::set<fd>::iterator it = server_fds.begin(); it != server_fds.end(); ++it)
    	close(*it);
	close(ep_fd);
	return (0);
}

// DANGER~ the most shittest function so far created "webserv.start()" better not touch it
// only gpt & I know it, now only I know what i'm gonna do. I will clean out later.
// I dont understand a shit at all. :) 25.Nov.2025/06:41