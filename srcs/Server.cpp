#include "../include/Server.hpp"


Server::Server(){

}

Server::Server(const Server &other){
    // *this = other;
    (void)other;
}

Server::~Server(){}

Server& Server:: operator=(const Server &other){
    // if(this != &other)
    //     *this = other;
    return *this;
    (void)other; 
}

void Server:: initiate(){
        struct sockaddr_in serverAddress;

        //creates an endpoint for communication
        //returns the file descriptor to that endpoint
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(8080);

        //bind socket(assign the address to the socket)
        if(bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) //on success return the 0
        {
            perror("something went wrong in binding");
        }
        std::cout << "socket is binded" << std::endl;


         //making of the non-blocking socket
        int flags = fcntl(serverSocket, F_GETFL, 0);
        if(flags < 0){
            perror("fcntl getting flag failed");
        }

        if(fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK) < 0){
            perror("fcntl non blocking failed");
        }

        //create epoll_instance
        int epoll_fd = epoll_create(5);
        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, NULL) < 0){
            std::cout << "adding into epoll failed" << std::endl;
        }
        std::cout << "added into epoll" << std::endl;

        //listen to the assigned socket
        if(listen(serverSocket, 5) < 0){
            std::cout << "server is not listening" << std::endl;
        }
         std::cout << "server is listening" << std::endl;
        
       

        //accept connection request
        //returns the file descriptor for the accepted socket
        int clientSocket = accept(serverSocket, NULL, NULL);
        std::cout << "accepting connection from client" << std::endl;
        char buffer[1024] = {0};
        //receiving data
        recv(clientSocket, buffer, sizeof(buffer), 0);
        close(serverSocket);
}
