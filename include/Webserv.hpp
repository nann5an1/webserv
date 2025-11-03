#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <map>

// enum allow_methods{
//    GET,
//    POST,
//    DELETE
// };

struct location_attr{
    bool    autoindex;
    int     get;
    int     post;
    int     del;
    std::string root;
    std::string upload_dir;
    std::vector<std::string> page_seq;
    std::vector<std::string> cgi;
};

class Webserv{
    private:
        std::string server_name;
        std::string listen_port;
        long long max_body_size;
        std::map<std::string, location_attr> location;
        std::map<std::string, std::string> err_pages;
    public:
        Webserv();
        ~Webserv();
        Webserv(const Webserv &other);
        Webserv& operator=(const Webserv &other);
        void watchServer();
        void fileParser(char *av);
};


#endif