#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Webserv.hpp"
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
        std::vector<std::map<std::string, std::string> > body;
    public:
        Request();
        ~Request();
        Request(const Request &other);
        Request &operator=(const Request &other);
        bool parseRequest(const char *raw_request);
};

#endif
