#include "Cgi.hpp"


Cgi::Cgi() {}

Cgi::~Cgi() {}


void Cgi::addToEnv(){
    // std::stringstream ss(cgi_env);
//     std::string token;

//     int i = 0;
    
//     while(ss >> token){
//         env[i] = strdup(token.c_str());
//         i++;
//     }
//     env[i] = NULL;
//     // int j = 0;
//     // while(env[j] != '\0'){
//     //     printf("%s\n", env[j++]);
//     // }
}


void Cgi::execute() {
    pid_t pid = fork();
    int pipefd[2];

    if(pipe(pipefd) < 0) std::cout << "pipe error" << std::endl;

    if(pid < 0){
        std::cout << "failed fork" << std::endl;
    }
    else if(pid == 0){

        //duplicate the fds
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        const char* file = "./test.py";
        
        char *argv[2];
        argv[0] = strdup(file);
        argv[1] = NULL;

        //for argv array there will only be 2 arguments(1st is the path from the config file + executable name, second is NULL)
        
        addToEnv(); //envp array
        if(execve(file, argv, env) < 0){
            std::cout << "failed execve" << std::endl;
        }
        std::cout << "this is child" << std::endl;
        
    }
    close(pipefd[1]);

    char buffer[4096];
    int n = read(pipefd[0], buffer, sizeof(buffer) - 1);
    buffer[n] = '\0';

    std::string cgi_output(buffer);
    std::cout << "CGI OUTPUT:\n" << cgi_output << std::endl;

    waitpid(pid, NULL, 0);
}


//split the string into env variables

