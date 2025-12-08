#ifndef CGI_HPP
#define CGI_HPP

#include <unistd.h>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <ostream>
#include <iostream>
#include <vector>
#include "Request.hpp"
#include <sys/wait.h>

class Cgi{
    private:
        std::string _env;
        char *env[50];
    public:
        Cgi();
        ~Cgi();
        // Cgi(std::string &exec_path);
        void execute();
        void addToEnv();
};


#endif