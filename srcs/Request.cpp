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


void Request::parseRequest(const char *raw_request){

}



