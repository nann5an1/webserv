#include "Server.hpp"

fd::fd() : fd_(-1) {}

fd::fd(int fd_) : fd_(fd_) {}

fd::operator int() const {return (fd_);}

Server::Server() {
}

Server::~Server(){
	// std::cout << "Server Destructor Called " << std::endl;
}

std::string Server::trimSemiColon(std::string val){
	if(val.find(";") == std::string::npos)	throw ConfigFileError();
	return (val.substr(0, val.length() - 1));
}

int Server::validateHTTPCode(std::string &val){
	for(size_t i = 0; i < val.length(); i++){
		if(!isdigit(val[i]))	return 0; //not digit
	}
	if(val.length() > 3 || val.length() < 3) return 0;
	return 1;
}

//get the data from the config file by line iteration
int Server::inputData(std::string &line){
	std::string token;  
	std::stringstream ss(line);
	// bool server_name = false, listen = false, root= true, max_body = true, err_page = true;
	// std::cout << "line in inputData >> " << line << std::endl;
	
	//line is the line by line
	while(ss >> token){
		std::string value;
		// std::cout << "token >> " << token << std::endl;
		if(token == "server_name")	{
			// std::string value;
			if(!(ss >> value)) return 0;
			else {
				
				this->_server_name = trimSemiColon(value);
				// std::cout << "server name debug >> " << this->_server_name << std::endl;
				break;
			}
			
		}
		else if(token == "listen"){ 
			// std::string value;
			if(!(ss >> value)) return 0;
			else{
				int idx = value.find(":");
				this->listen_ip  = value.substr(0, idx);
				this->listen_port = trimSemiColon(value.substr(idx + 1, value.length() - idx - 1));
				break;
				// std::cout << "listen >> " << this->listen_ip << ":" << this->listen_port << std::endl;
			}
		}
		else if(token == "root"){
			if(!(ss >> value)) return 0;
			else {
				this->root = trimSemiColon(value);
				break;
			}
		}
		else if(token == "max_body_size"){
			if(!(ss >> value)) return 0;
			else{
				this->max_body_size = atoi(value.c_str());
				break;
			}
		}
		else if(token == "error_page"){
			
			ss >> value;
			// std::cout << "error page status code >> " << value << std::endl;
			if(!validateHTTPCode(value)) return 0;
			else{ //http code is validated
				std::string path;
				if(!(ss >> path)) return 0;
				else{
					this->err_pages.insert(std::pair<int, std::string>(atoi(value.c_str()), trimSemiColon(path)));
					// break;
				}
			}
		}
		else{
			break;
		}
	}
	
	
	return 1;	
}

int Server::inputLocation(std::string line, t_location &location){
	std::string token;
	std::stringstream ss(line);

	while(ss >> token){
		// std::cout << "token  in inputLocation>> " << token << std::endl;
		std::string val;
		if(token == "autoindex"){
			ss >> val;
			val = trimSemiColon(val);
			if(val == "on") location.autoindex = true;
			else location.autoindex = false;
		}
		else if(token == "methods"){
			while(ss >> val){
				if(val.find(";") != std::string::npos)	val = trimSemiColon(val);

				if(val == "GET")	location.get = true;
				if(val == "POST")	location.post = true;
				if(val == "DELETE")	location.del = true;
			}
			// std::cout << "methods >> " << method1 << " " << method2 << " " << method3 << std::endl;	
		}
		else if(token == "root"){
			ss >> val;
			val = trimSemiColon(val);
			location.root = val;
			// std::cout <<"location root >> " << location.root << std::endl;
		}
		else if(token == "upload_dir"){
			ss >> val;
			val = trimSemiColon(val);
			location.upload_dir = val;
		}
		else if(token == "index"){
			while(ss >> val)
				location.index_files.push_back(val);
			// std::cout << location.index_files[0] << " " << location.index_files[1] << std::endl;
		}
		else if(token == "cgi"){
			std::string key, val;
			ss >> key >> val;
			val = trimSemiColon(val);
			location.cgi.insert(std::pair<std::string, std::string>(key, val));
		}
		else if(token == "return"){
			std::string key, val;
			ss >> key >> val;
			val = trimSemiColon(val);
			if(!validateHTTPCode(key)) return 0;
			location.ret_pages.insert(std::pair<int, std::string>(atoi(key.c_str()), val));
		}
	}
	
	return 1;
}


//will retrive the strating from the next line of the server scope
Server::Server(std::ifstream &file, int serv_scope_start)
: _server_name("default"), 
  listen_port("default"),
  listen_ip("default"),
  root("default"),
  max_body_size(0),
  location_path("default")
{
	// std::cout << "Server parameterized constructor" << std::endl;
	std::string line, tok;
	
	int location_scope = 1, location_key = 1;
	int server_scope = serv_scope_start;
	t_location location;

	// std::cout << "servere scope count >> " << server_scope << std::endl;
	//going line by line inside the server scope now
	while(getline(file, line) && server_scope > 1){
		std::stringstream ss(line);
		// std::cout << "line now at >> " << line << std::endl;
		while(ss >> tok){
			if(tok == "location"){
				location = t_location();
				std::string path, scope;
				ss >> path >> scope;
				
				this->location_path = path;
				location_key++;
				if(scope == " ") break;
				else location_scope++;
			}
			else if(tok == "{"){
				location_scope++;
				// std::cout << "location scope start" << location_scope << std::endl;
			}
			else if(server_scope > 1 && location_scope <= 1 && tok == "}"){
				server_scope--;
			}
			else if(tok == "}"){
				location_scope--;
				location_key--;
				this->location_map.insert(std::pair<std::string, t_location>(this->location_path, location));
				this->location_path = "default";
			}
			
		}
		if(server_scope > 1){ //inside the scope 
			//parsing will come in
			
			// std::cout << "line under server scope >> " << line << std::endl;
			if(inputData(line) != 1) throw ConfigFileError();
			// std::cout << "location scope count" << location_scope << std::endl;
			if(location_scope > 1){
				if (line.find("{") != std::string::npos ||
					line.find("}") != std::string::npos ||
					line.find("location") != std::string::npos)
					continue; 
				if(!inputLocation(line, location)) throw ConfigFileError();
			}
		}
		else if(server_scope <= 1)	break;
	}
	// std::cout << "server name >> " << this->_server_name << "\n"
	// 			<< "listen >> " << this->listen_ip << ":" << this->listen_port << "\n"
	// 			<< "root >> " << this->root << "\n"
	// 			<< "max_body_size >> " << this->max_body_size << "\n" << std::endl;

	this->start();
	
}

Server::Server(const Server &other):
	_server_name(other._server_name),
	listen_port(other.listen_port),
	listen_ip(other.listen_ip),
	root(other.root),
	max_body_size(other.max_body_size),
	location_path(other.location_path),
	location_map(other.location_map),
	err_pages(other.err_pages)

{
    (void)other;
}


Server& Server:: operator=(const Server &other) {
    // if(this != &other)
    //     *this = other;
	(void)other; 
    return *this;
}

int	Server::start()
{
	socklen_t	opt = 1;
	struct sockaddr_in	sock_addr;

	std::memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;	
	// TEMP~ I set the port to 8080 for test.
	sock_addr.sin_port = htons(std::atoi(listen_port.c_str()));
	sock_addr.sin_addr.s_addr = inet_addr(listen_ip.c_str());
	if ((_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ||
		setsockopt(_sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0 ||
		bind(_sock_fd, (sockaddr *)&sock_addr, sizeof(sock_addr)) < 0 ||
		fcntl(_sock_fd, F_SETFL, fcntl(_sock_fd, F_GETFL, 0) | O_NONBLOCK) < 0 ||
		listen(_sock_fd, SOMAXCONN) < 0)
	{
		int status = fail("Server", errno);
		if (_sock_fd > 0)
			close(_sock_fd);
		_sock_fd = -1;
		return (status);
	}
	std::cout << "Server: '" << std::string(*this) << "'\tstarted with socket: " << _sock_fd << std::endl;
	return (0);
}

Server::operator	fd() const
{
	return (_sock_fd);
}

Server::operator	int() const
{
	return (std::atoi(listen_port.c_str()));
}

Server::operator	std::string() const
{
	return (listen_ip + ":" + listen_port + ":" + _server_name);
}

ConfigFileError::ConfigFileError()
	: std::runtime_error("Error in config file") {}



//print out the values inside the server
void Server::print() const {
    std::cout << "==================== SERVER ====================\n";
    std::cout << "Server Name: " << _server_name << "\n";
	std::cout << "Root: " << root << "\n";
    std::cout << "IP: " << listen_ip << "\n";
    std::cout << "Port: " << listen_port << "\n";
    std::cout << "Max Body Size: " << max_body_size << "\n";

    std::cout << "\n-- Error Pages --\n";
    for (std::map<int, std::string>::const_iterator it = err_pages.begin(); it != err_pages.end(); ++it) {
        std::cout << it->first << " => " << it->second << "\n";
    }

    std::cout << "\n-- Locations --\n";
    for (std::map<std::string, t_location>::const_iterator it = location_map.begin(); it != location_map.end(); ++it) {
        const std::string &path = it->first;
        const t_location &loc = it->second;

        std::cout << "Location: " << path << "\n";
        std::cout << "  autoindex: " << (loc.autoindex ? "on" : "off") << "\n";
        std::cout << "  methods: "
                  << (loc.get ? "GET " : "")
                  << (loc.post ? "POST " : "")
                  << (loc.del ? "DELETE " : "") << "\n";
        std::cout << "  root: " << loc.root << "\n";
        std::cout << "  upload_dir: " << loc.upload_dir << "\n";

        std::cout << "  index files: ";
        for (size_t i = 0; i < loc.index_files.size(); i++)
            std::cout << loc.index_files[i] << " ";
        std::cout << "\n";

        std::cout << "  CGI: ";
        for (std::map<std::string, std::string>::const_iterator ci = loc.cgi.begin(); ci != loc.cgi.end(); ++ci)
            std::cout << ci->first << " => " << ci->second << " ";
        std::cout << "\n";

        std::cout << "  Return pages: ";
        for (std::map<int, std::string>::const_iterator rp = loc.ret_pages.begin(); rp != loc.ret_pages.end(); ++rp)
            std::cout << rp->first << " => " << rp->second << " ";
        std::cout << "\n\n";
    }
    std::cout << "=================================================\n";
}

int	fail(std::string head, int err_no)
{
	std::cerr << RED << "Error: " << head << ": " << strerror(err_no) << RESET << std::endl;
	return (err_no);
}
