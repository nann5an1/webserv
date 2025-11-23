#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>

// this is simple testing of two servers connected with mulitple clients and sending simple response using epoll

#define MAX_EVENTS 10

int make_server(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); exit(1); }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(sock, 5) < 0) { perror("listen"); exit(1); }

    fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);

    return sock;
}

int main() {
    // Create two servers
    int server1 = make_server(8080);
    int server2 = make_server(8081);

    int epfd = epoll_create1(0);
    if (epfd < 0) { perror("epoll_create"); exit(1); }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    
    ev.data.fd = server1;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server1, &ev);

    ev.data.fd = server2;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server2, &ev);

    struct epoll_event events[MAX_EVENTS];

    std::cout << "Servers running on ports 8080 and 8081..." << std::endl;

    while (true) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (n < 0) { perror("epoll_wait"); break; }

        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;

            if (fd == server1 || fd == server2) {
                // New client connection
                int client_fd = accept(fd, NULL, NULL);
                if (client_fd < 0) continue;
                fcntl(client_fd, F_SETFL, fcntl(client_fd, F_GETFL, 0) | O_NONBLOCK);

                ev.events = EPOLLIN;
                ev.data.fd = client_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);

                std::cout << "New client connected to server on port "
                          << ((fd == server1) ? 8080 : 8081)
                          << ", fd=" << client_fd << std::endl;
            } else {
                // Existing client has data
                char buf[1024];
                int bytes = read(fd, buf, sizeof(buf));
                if (bytes <= 0) {
                    // Client closed
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    std::cout << "Client disconnected, fd=" << fd << std::endl;
                } else {
                    // Echo back a simple response
                    std::cout << "Received: " << std::string(buf, bytes)
                              << " from fd=" << fd << std::endl;
                    const char* reply = "Hello from server!\n";
                    write(fd, reply, strlen(reply));
                }
            }
        }
    }

    close(server1);
    close(server2);
    close(epfd);
    return 0;
}


