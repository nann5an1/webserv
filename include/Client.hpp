#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>

class Client{
    Client();
    ~Client();
    Client(const Client &other);
    Client& operator=(const Client &other);
};

#endif