#ifndef CGI_HPP
#define CGI_HPP

#include <unistd.h>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <ostream>
#include <iostream>
class CGI{
    public:
        CGI();
        ~CGI();
        void execute();
};


#endif