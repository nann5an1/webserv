#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
// #include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>  // For errno
#include <cstddef>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>

class Server{
    private:
        // int socket_fd; //endpoint for communication
    public:
        Server();
        Server(const Server &other);
        ~Server();
        Server &operator=(const Server &other);
        void initiate();
};

#endif