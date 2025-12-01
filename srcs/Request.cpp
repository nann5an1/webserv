#include "Request.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>

Request::Request() {}
Request::~Request() {}
Request::Request(const Request &other):
    method(other.method),
    path(other.path),
    hostname(other.hostname),
    port(other.port),
    conn_status(other.conn_status),
    body(other.body)
{
    *this = &other;
}

Request &Request::operator=(const Request &other){
    (void)other;
    return *this;
}

bool validate_len(std::string token){
    for(size_t i = 0; i < token.length(); i++){
        if(!isdigit(token[i]))  return false;
    }
    return true;
}

void Request::parseRequest(const char *raw_request){
    bool hostname = false;
    bool bool_content_len = false;
    bool bool_connection = false;
    std::string header = raw_request;
    while(std::getline(header, line)){
        std::stringstream stream(line);
        std::string token;
        while(stream >> token){
            if(token == "POST" || token == "GET"  || token == "DELETE"){
                this->method = token;
            }
            else if(token[0] == "/")    this->path = token;
            else if(token == "HTTP/1.1") this->version = token;
            else if(token == "Host:") hostname = true;
            else if(hostname){
                //will be the value after the host
                int idx = token.find(":") //index of the :
                this->hostname = token.substr(0, idx);
                this->port = token.substr(idx + 1, token.length() - idx - 1);
            }
            else if(token == "Content-Length") bool_content_len = true;
            else if(bool_content_len){
                if(validate_len(token)) this->content_len = std::stoi(token);
                bool_content_len = false;
            }
            else if(token == "Connection") bool_connection = true;
            else if(bool_connection){
                this->conn_status = token;
            }
        }
    }
}



