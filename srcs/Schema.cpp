#include "Schema.cpp"
#include "Request.hpp"

Schema::Schema(){}


Schema::~Schema(){}

Schema::Schema(int request_cat){
    switch (request_cat)
    {
    case 0:
        // CGI cgi("/bin/bash");
        // cgi.execute();
        // break;
    default:
        break;
    }
    std::cout << request_category << std::endl;
}