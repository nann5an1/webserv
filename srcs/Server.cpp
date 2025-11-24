#include "Server.hpp"

fd::fd() : FD(-1) {}

fd::fd(int fd_) : FD(fd_) {}

fd::operator int() const {return (FD);}

Server::Server() {
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
int Server::inputData(std::string line){
	std::string token, value;  
	std::stringstream ss(line);
	ss >> token >> value;

	//change the string into the int -- don't forget
	if(token == "server_name")	this->server_name = value;
	else if(token == "listen"){
		int idx = value.find(":");
		this->listen_ip  = value.substr(0, idx);
		this->listen_port = value.substr(idx + 1, value.length() - idx - 1);
	}
	else if(token == "error_page"){
		std::string path;
		ss >> path;
		// std::cout << "path >> " << path << std::endl;
		path = trimSemiColon(path);
		if(!validateHTTPCode(value)) return 0;
		this->err_pages.insert(std::pair<int, std::string>(atoi(value.c_str()), path));
	}
	else if(token == "client_max_body_size")	this->max_body_size = atoi(value.c_str());

	return 1;	
}

int Server::inputLocation(std::string line, t_location &location){
	std::string token;
	
	std::stringstream ss(line);
	ss >> token;

	if(token == "autoindex"){
		std::string val;
		ss >> val;
		val = trimSemiColon(val);
		if(val == "on") location.autoindex = true;
	}
	else if(token == "allow_methods"){
		std::string method1, method2, method3;
		ss >> method1 >> method2 >> method3;
		// std::cout << "methods >> " << method1 << " " << method2 << " " << method3 << std::endl;

		if(method1.find(";") != std::string::npos)	method1 = trimSemiColon(method1);
		if(method2.find(";") != std::string::npos)	method2 = trimSemiColon(method2);
		if(method3.find(";") != std::string::npos)	method3 = trimSemiColon(method3);

		if(method1 == "GET")	location.get = true;
		if(method2 == "POST")	location.post = true;
		if(method3 == "DELETE")	location.del = true;
	}
	else if(token == "root"){
		std::string val;
		ss >> val;
		val = trimSemiColon(val);
		location.root = val;
	}
	else if(token == "upload_dir"){
		std::string val;
		ss >> val;
		val = trimSemiColon(val);
		location.upload_dir = val;
	}
	else if(token == "index"){
		std::string val1, val2;
		ss >> val1 >> val2;
		location.index_files.push_back(val1);
		location.index_files.push_back(val2);
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
	return 1;
}


//will retrive the strating from the next line of the server scope
Server::Server(std::ifstream &file, int server_scope){
	std::string line, tok;
	std::stringstream ss(line);
	int location_scope = 1, location_key = 1;
	t_location location;

	//going line by line inside the server scope now
	while(getline(file, line) && server_scope > 1){
		while(ss >> tok){
			if(tok == "{") {
				server_scope++;
				break;
			}
			else if(tok == "location"){
				location_key++;
			}
			else if(location_key > 1 && tok == "{"){
				location_scope++;
			}
			else if(location_key > 1){ //just a simple path parsing for the location
				this->location_path = tok;
			}
			else if(server_scope > 1 && tok == "}"){
				server_scope--;
			}
		}
		if(server_scope > 1){ //inside the scope 
			//parsing will come in
			if(!inputData(line)) throw ConfigFileError();
			if(location_scope > 1){
				if(!inputLocation(line, location)) throw ConfigFileError();
			}
		}
		else if(server_scope <= 1)	break;
	}
}

Server::Server(const Server &other) {
    (void)other;
}

Server::~Server() {}

Server& Server:: operator=(const Server &other) {
    // if(this != &other)
    //     *this = other;
	(void)other; 
    return *this;
}

int	Server::start()
{
	int	err = 0;
	socklen_t	opt = 1;
	struct sockaddr_in	sock_addr;

	std::memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;	
	// TEMP~ I set the port to 8080 for test.
	listen_port = "8080";
	listen_ip = "127.0.0.1";
	server_name = "nsan.server";
	sock_addr.sin_port = htons(std::atoi(listen_port.c_str()));
	sock_addr.sin_addr.s_addr = inet_addr(listen_ip.c_str());
	if ((_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ||
		setsockopt(_sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0 ||
		bind(_sock_fd, (sockaddr *)&sock_addr, sizeof(sock_addr)) < 0 ||
		fcntl(_sock_fd, F_SETFL, fcntl(_sock_fd, F_GETFL, 0) | O_NONBLOCK) < 0 ||
		listen(_sock_fd, SOMAXCONN) < 0)
	{
		err = errno;
		if (_sock_fd > 0)
			close(_sock_fd);
		_sock_fd = -1;
		std::cout << "Err: " << err << std::endl;
		return (err);
	}
	sleep(3);
	std::cout << "Server.start(): " << _sock_fd << std::endl;
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
	return (listen_ip + ":" + listen_port + ":" + server_name);
}

ConfigFileError::ConfigFileError()
	: std::runtime_error("Error in config file") {}

// void Server::initiate() {

//         // struct sockaddr_in serverAddress;

//         // //creates an endpoint for communication
//         // //returns the file descriptor to that endpoint
//         // int listenfd = socket(AF_INET, SOCK_STREAM, 0);

//         // serverAddress.sin_family = AF_INET;
//         // serverAddress.sin_addr.s_addr = INADDR_ANY;
//         // serverAddress.sin_port = htons(8080);

//         // //bind socket(assign the address to the socket)
//         // if(bind(listenfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) //on success return the 0
//         // {
//         //     perror("something went wrong in binding");
//         // }
//         // std::cout << "socket is binded" << std::endl;


//         //  //making of the non-blocking socket
//         // int flags = fcntl(listenfd, F_GETFL, 0);
//         // if(flags < 0){
//         //     perror("fcntl getting flag failed");
//         // }

//         // if(fcntl(listenfd, F_SETFL, flags | O_NONBLOCK) < 0){
//         //     perror("fcntl non blocking failed");
//         // }

//         // //create epoll_instance
//         // // int epoll_fd = epoll_create(5);
//         // // if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, NULL) < 0){
//         // //     std::cout << "adding into epoll failed" << std::endl;
//         // // }
//         // // std::cout << "added into epoll" << std::endl;

//         // int epfd = epoll_create1(0);

//         // struct epoll_event ev;
//         // ev.events = EPOLLIN;
//         // ev.data.fd = listenfd;
//         // // epoll_ctl(epfd, EPOLL_CTL_ADD, serverSocket, &ev);

//         // epoll_event events[1024];
//         // while(true){
//         //     int n = epoll_wait(epfd, events, 1024, -1);
//         //     if(n == -1){
//         //         perror("epoll wait");
//         //         break;
//         //     }
//         //     for(int i = 0; i < n; i++){
//         //         int fd = events[i].data.fd;
//         //         if(fd == listenfd){
//         //             epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
//         //         } //not an initial connection(handle request)
//         //         else if(EPOLLIN)
//         //         {
                        
//         //         }
//         //         else //handle response back to the client
//         //         {

//         //         }
//         //     }
//         // }

//         // //listen to the assigned socket
//         // if(listen(listenfd, 5) < 0){
//         //     std::cout << "server is not listening" << std::endl;
//         // }
//         //  std::cout << "server is listening" << std::endl;
        
        
        

//         // //accept connection request
//         // //returns the file descriptor for the accepted socket
//         // int clientSocket = accept(listenfd, NULL, NULL);
//         // std::cout << "accepting connection from client" << std::endl;
//         // char buffer[1024] = {0};
//         // //receiving data
//         // recv(clientSocket, buffer, sizeof(buffer), 0);
//         // close(listenfd);
// }
