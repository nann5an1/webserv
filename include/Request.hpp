#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Webserv.hpp"
#include "Server.hpp"
#include <iostream>
#include <map>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <sstream>

class Request{
    private:
        std::string method;
        std::string path;
        std::string version;
        std::string hostname;
        int port;
        std::string content_type;
        int content_len;
        std::string conn_status;
        std::map<std::string, Server> attachServer;
    public:
        Request();
        ~Request();
        Request(const Request &other);
        Request &operator=(const Request &other);
        void parseRequest(const char *raw_request);
        void fetchServerScope();
};

#endif
