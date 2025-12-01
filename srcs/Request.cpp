#include "Request.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <string>

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
    *this = other;
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

std::string toLower(std::string token){
    std::string result = "";
    for(size_t i = 0; i < token.length(); i++){
        if(isalpha(token[i]))  result += (char)std::tolower(token[i]);
        else result += token[i];
    }
    return result;
}

//----------------- request parsing --------------------
void Request::parseRequest(const char *raw_request){
    bool hostname = false;
    bool bool_content_len = false;
    bool bool_connection = false;
    std::string header = raw_request;
    std::string line;
    std::istringstream iss(header);

    //process each header line
    while(std::getline(iss, line)){
        std::stringstream stream(line);
        std::string token;
        // std::cout << "Tokens" << std::endl;
        while(stream >> token){ //toLower is applied because headers are case-insensitive
            // std::cout << token << std::endl;
            if(token == "POST" || token == "GET"  || token == "DELETE") this->method = token;
            else if(token[0] == '/')    this->path = token;
            else if(token == "HTTP/1.1") this->version = token;
            else if(toLower(token)  == "host:") hostname = true;
            else if(toLower(token)  == "content-length:") bool_content_len = true;
            else if(toLower(token)  == "connection:") bool_connection = true;
            else if(bool_content_len){
                if(validate_len(token)) this->content_len = atoi(token.c_str());
                bool_content_len = false;
            }
            else if(hostname){
                //will be the value after the host
                int idx = token.find(":"); //index of the ':'
                this->hostname = token.substr(0, idx);
                this->port = atoi(token.substr(idx + 1, (token.length()) - idx - 1).c_str());
                hostname = false;
            }
            else if(bool_connection){
                this->conn_status = token;
                bool_connection = false;
            }
        }
    }

    std::cout << "-------- Request parsing -------" << "\n"
              << "Method >> " << this->method << "\n"
              << "Hostname >> " << this->hostname << "\n"
              << "Port >> " << this->port << "\n"
              << "Content-Length >> " << this->content_len << "\n"
              << "Connection >> " << this->conn_status << "\n"
              << std::endl;
}

//-------------------- fetch the correct server scope from webserv ----------------
void Request::fetchServerScope(){

}

