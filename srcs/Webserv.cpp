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
    std::string line;
    location_attr.autoindex = false;
    
    if(file.is_open())
    {
        std::cout << "File is opened now" << std::endl;

        //get the first line of the file to check if it has the word "server" for scope check
        getline(file, line);

        // std::cout << "line >> " << line << std::endl; 
        if((line.length() > 6 || line.length() < 0) || line != "server"){
            perror("the config file is not in server scope");
            return ;
        }
        while(getline(file, line))
        { //read line by line
                std::string token1, token2, token3;
                std::stringstream ss(line);
                if(line.find("error_page") != std::string::npos){
                        while (ss >> token1 >> token2 >> token3){
                        this->err_pages.insert(std::pair<std::string, std::string>(token2, token3));
                    }
                }
                else if(line.find("location") != std::string::npos){
                    while(ss >> token1 >> token2 >> token3){
                        location_attr.path = token2;
                        break;
                    }
                }
                else if(line.find("autoindex") != std::string::npos){
                    while(ss >> token1 >> token2){
                        if(token2 == "on")  location_attr.autoindex = true;
                    }
                }
                else if(line.find("allow_methods") != std::string::npos){
                    while(ss >> token1 >> token2 >> token3){
                        if(token1 == "GET" )location_attr.get = true;
                        else if(token2 == "POST") location_attr.post = true;
                        else if(token3 == "DELETE") location_attr.del = true;
                    }
                }
                else if(line.find("root") != std::string::npos){
                    while(ss >> token1 >> token2){
                        location_attr.root = token2;
                        break;
                    }
                }
                else if(line.find("cgi") != std::string::npos){
                    while(ss >> token1 >> )
                }
                while(ss >> token1 >> token2){
                    if(token1 == "server_name") this->server_name = token2;
                    else if(token1 == "listen") this->listen_port = token2;
                    else if(token1 == "client_max_body_size")   this->max_body_size = atol(token2.c_str());
                }
        }
    }
    std::cout << "\n<< Values check >> \nserver name >> " <<  this->server_name << "\n"
    << "listen >> " << this->listen_port
    << "\nclient max body size >> " << this->max_body_size << std::endl;
    print_map();
    std::cout << "location's path << " << location_attr.path;
}


void Webserv::print_map(){
    //cloned copy for iteration
    std::map<std::string, std::string>err_pages_copied;

    // this->err_pages.insert(std::pair<std::string, std::string>(line.substr(firstdeli + 1, 3), line.substr(lastdeli + 1, line.length() - lastdeli)));

    //iterate the error_pages in the map
    err_pages_copied = this->err_pages;
    
    for(std::map<std::string, std::string>::iterator it = err_pages_copied.begin(); it != err_pages_copied.end(); ++it){
        std::cout << it->first << " " << it->second << std::endl;
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

