#include "../include/Webserv.hpp"

Webserv::Webserv(){}

Webserv::~Webserv(){}
Webserv::Webserv(const Webserv &other){
    (void)other;
}

Webserv& Webserv:: operator=(const Webserv &other){
    (void)other;
    return *this;
}

void Webserv::watchServer(){
    // while(epoll())
}

void Webserv::fileParser(char *av){
    std::string word;
    std::string config_file;
    if(av) config_file = av;
    else config_file = "def.conf";

    std::ifstream file(config_file.c_str());

    // std::cout << config_file << std::endl;
    std::string line;

    if(file.is_open()){
        std::cout << "File is opened now" << std::endl;
            while(getline(file, line)){ //read line by line
                if(line.find("server") != std::string::npos){
                    std::string token1, token2, token3;
                    std::stringstream ss(line);
                    while (ss >> token1 >> token2 >> token3){
                        if(token1 == "server_name") this->server_name = token2;
                        else if(token1 == "listen") this->listen_port = token2;
                        else if(token3){
                            if(token1 == "error_page"){
                                this->err_pages.insert(std::pair<std::string, std::string>(token2, token3));
                            }
                        }
                    }
                }

                // if(line.find("server_name") != std::string::npos){
                //     int space = line.find(" ");
                //     this->server_name = line.substr(space + 1, line.length() - space);
                //     std::cout << this->server_name << std::endl;
                // }
                // else if(line.find("listen") != std::string::npos){
                //     int space = line.find(" ");
                //     this->listen_port = line.substr(space + 1, line.length() - space);
                // }
                // else if(line.find("error_page") != std::string::npos){
                //     int firstdeli = line.find(" ");
                //     int lastdeli = line.rfind(" ");

                    //cloned copy for iteration
                    std::map<std::string, std::string>err_pages_copied;

                    // this->err_pages.insert(std::pair<std::string, std::string>(line.substr(firstdeli + 1, 3), line.substr(lastdeli + 1, line.length() - lastdeli)));

                    //iterate the error_pages in the map
                    err_pages_copied = this->err_pages;
                    
                    // for(std::map<std::string, std::string>::iterator it = err_pages_copied.begin(); it != err_pages_copied.end(); ++it){
                    //     std::cout << it->first << " " << it->second << std::endl;
                    // }
                }
                else if(line.find("client_max_body_size") != std::string::npos){
                    this->max_body_size = 
                }
            }
    }
}