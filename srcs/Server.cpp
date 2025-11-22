#include "Server.hpp"

Server::Server() {

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
	try
	{
		if ((_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			throw	std::runtime_error("creation failed!");
		socklen_t	opt = 1;
		if (setsockopt(_sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
			throw	std::runtime_error("creation failed!");
		
		sockaddr_in	sock_addr;
		std::memset(&sock_addr, 0, sizeof(sock_addr));
		sock_addr.sin_family = AF_INET;	
		// TEMP: I set the port to 8080 for test.
		listen_port = 8080;
		sock_addr.sin_port = htons(std::atoi(listen_port.c_str()));
		sock_addr.sin_addr.s_addr = inet_addr(listen_ip.c_str());
		if (bind(_sock_fd, (sockaddr *)&sock_addr, sizeof(sock_addr)) < 0)
		{
			close(_sock_fd);
			throw	std::runtime_error("bind error!");
		}
		if (fcntl(_sock_fd, F_SETFL, fcntl(_sock_fd, F_GETFL, 0) | O_NONBLOCK))
		{
			close(_sock_fd);
			throw	std::runtime_error("non-blocking error!");
		}
		if (listen(_sock_fd, SOMAXCONN) < 0)
		{
			close(_sock_fd);
			throw	std::runtime_error("listen failed!");
		}
	} catch(std::exception &e)
	{
		std::cerr << RED << "Error: Socket: " << e.what() << std::endl;
		return (0);
	}
	return (1);
}

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
