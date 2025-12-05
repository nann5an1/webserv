#include "CGI.hpp"

CGI::CGI() {}

CGI::~CGI() {}

void CGI::execute() {
    pid_t pid = fork();
    if(pid < 0){
        std::cout << "failed fork" << std::endl;
    }
    else if(pid == 0){
        std::cout << "this is child" << std::endl;
    }
}