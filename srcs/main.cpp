#include "Webserv.hpp"
#include "Server.hpp"

int main(int ac, char **av){
    if(ac > 2) return 1;
    
    Webserv webserv;
    webserv.fileParser(av[1]);
    webserv.watchServer();

    Server server;
    server.initiate();
}