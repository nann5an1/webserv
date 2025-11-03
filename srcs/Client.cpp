#include "../include/webserv.hpp"
#include "../include/Client.hpp"

Client::Client(){}

Client::~Client(){}

Client::Client(const Client &other){
    (void)other;
}

Client& Client::operator=(const Client &other){
    (void)other;
    return *this;
}