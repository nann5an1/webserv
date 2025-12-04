#ifndef REQUEST_HPP
#define REQUEST_HPP

// #include "Webserv.hpp"
// #include "Server.hpp"
#include <iostream>
#include <map>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <sstream>


enum content_category{
    ERROR,
    JSON,
    URLENCODED,
    FORM
};

class Request{
    private:
        std::string method;
        std::string path;
        std::string version;
        std::string hostname;
        int port;
        int content_type;
        int content_len;
        std::string conn_status;
        std::string query;
        std::string body;
        bool bool_cgi;
        bool bool_boundary;
        bool bool_referer;
        bool bool_binary;
        std::string cgi_env;
        std::string boundary;
        std::string referer;
        std::string filename;
        std::string binary_data;
        std::map<std::string, content_category> content_types;
        // std::map<std::string, Server> attachServer;
    public:
        Request();
        ~Request();
        Request(const Request &other);
        // Request &operator=(const Request &other);
        void parseRequest(const char *raw_request);
        void fetchServerScope();
        void read_binary(std::string &line);
};

#endif
