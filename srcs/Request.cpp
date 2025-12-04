#include "Request.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <string>

Request::Request():
    method(""),
    path(""),
    version(""),
    hostname(""),
    port(0),
    content_type(-1),
    content_len(0),
    conn_status(""),
    body(""),
    filename(""),
    bool_cgi(false),
    bool_boundary(false),
    bool_referer(false),
    bool_binary(false),
    cgi_env("")
{
    content_types["application/json"] = JSON;
    content_types["application/x-www-form-urlencoded"] = URLENCODED;
    content_types["multipart/form-data"] = FORM;
}

Request::~Request() {}
// Request::Request(const Request &other):
//     method(other.method),
//     path(other.path),
//     hostname(other.hostname),
//     port(other.port),
//     conn_status(other.conn_status),
//     body(other.body)
// {
//     *this = other;
// }

// Request &Request::operator=(const Request &other){
//     (void)other;
//     return *this;
// }

bool validate_len(std::string token){
    for(size_t i = 0; i < token.length(); i++){
        if(!isdigit(token[i]))  return false;
    }
    return true;
}

std::string toLower(std::string token){
    std::string result = "";
    for(size_t i = 0; i < token.length(); i++){
        if(isalpha(token[i]))  result += (char)std::tolower(token[i]);
        else result += token[i];
    }
    return result;
}

void Request::read_binary(std::string &line){
    std::stringstream stream(line);
    std::string sentence;

    while(std::getline(stream, sentence) &&
    sentence.find(this->boundary) != std::string::npos){
        this->body += sentence;
    }
}


//----------------- request parsing --------------------
void Request::parseRequest(const char *raw_request){
    // std::map<std::string, content_category> content_types;
    
    bool_cgi = false;
    bool hostname = false;
    bool bool_content_len = false;
    bool bool_connection = false;
    bool bool_cont_type = false;

    size_t idx2;
    int length;
    std::string header = raw_request;
    std::string line;
    std::istringstream iss(header);

    //process each header line
    while(std::getline(iss, line)){
        std::stringstream stream(line);
        std::string token;
        
        if(bool_boundary && line.empty()) bool_binary = true;
        while(stream >> token){ //toLower is applied because headers are case-insensitive
            // std::cout << token << std::endl;
            if(token == "POST" || token == "GET"  || token == "DELETE") this->method = token;
            else if(token[0] == '/'){
                size_t idx = token.find(".");
                size_t queryIdx = token.find("?");
                // std::cout << "idx >> " << idx << std::endl;
                if(idx != std::string::npos){ //if the extension for the cgi is true
                    idx2 = token.find("?");
                    // std::cout << "idx2 >> " << idx2 << std::endl;
                    if(idx2 == std::string::npos) {
                        idx2 = 0;
                        length = token.length() - idx - 1;
                    }
                    else length = idx2 - idx - 1;
                    // std::cout << "ext length >> " << length << std::endl;
                    std::string ext = token.substr(idx + 1, length);
                    // std::cout << "extension >> " << ext << std::endl;
                    if(ext == "php" || ext == "py" || ext == "pl" || ext == "sh" || ext == "rb") this->bool_cgi = true;
                }
                if(this->method == "GET" && queryIdx != std::string::npos){ //query parsing
                    this->query = token.substr(queryIdx + 1, token.length() - queryIdx);
                    // std::cout << "query >> " << query << std::endl;
                    this->cgi_env += query + "\n";
                }
                this->path = token;
                
            }
            else if(token == "HTTP/1.1") this->version = token;
            else if(toLower(token)  == "host:") hostname = true;
            else if(toLower(token)  == "content-length:") bool_content_len = true;
            else if(toLower(token)  == "connection:") bool_connection = true;
            else if(toLower(token) == "content-type:") bool_cont_type = true;
            else if(toLower(token) == "referer:") bool_referer = true;
            else if(bool_content_len){
                if(validate_len(token)) this->content_len = atoi(token.c_str());
                this->cgi_env += "CONTENT_LENGTH=" + token + "\n";
                bool_content_len = false;
            }
            else if(hostname){
                //will be the value after the host
                int idx = token.find(":"); //index of the ':'
                this->hostname = token.substr(0, idx);
                token = token.substr(idx + 1, (token.length()) - idx - 1).c_str();
                this->port = atoi(token.c_str());
                this->cgi_env += "SERVER_PORT=" + token + "\n";
                hostname = false;
            }
            else if(bool_connection){
                this->conn_status = token;
                bool_connection = false;
            }
            else if(bool_cont_type){
                if(!this->content_types[token])
                    std::cout << "error content type" << std::endl;
                this->content_type = this->content_types[token];
                
                // std::cout << "content type >> " << content_type << std::endl;
                this->cgi_env += "CONTENT_TYPE=" + token + "\n";
                bool_cont_type = false;
            }
            else if(token.find("boundary=") != std::string::npos){
                int idx = token.find("=");
                this->boundary = token.substr(idx + 1, token.length() - idx - 1);
                std::cout << "boundary >> " << this->boundary << std::endl;
            }
            else if(token == this->boundary){
                bool_boundary = true;
            }
            else if(token.find("filename") != std::string::npos){
                int firstIdx = token.find("\"");
                int lastIdx = token.length() - 1;
                this->filename = token.substr(firstIdx + 1, lastIdx - firstIdx - 1);

            }
            else if(bool_referer){
                this->referer = token;
                bool_referer = false;
            }
            else if(!bool_boundary && token.find("=") != std::string::npos){ //body parsing
                this->body = token;
            }
        }
        if(bool_binary){
            read_binary(line);
        }
    }

    if(bool_cgi){
        this->cgi_env += "REQUEST_METHOD=" + this->method + "\n" +
                         "QUERY_STRING=" + this->query + "\n" +
                         "SERVER_NAME=" + this->hostname + "\n" +
                         "SERVER_PROTOCOL=" + this->version + "\n" +
                         "SCRIPT_NAME=" + this->path + "\n";
    }
    std::cout << "-------- Request parsing -------" << "\n"
              << "Method >> " << this->method << "\n"
              << "Hostname >> " << this->hostname << "\n"
              << "Port >> " << this->port << "\n"
              << "Content-Length >> " << this->content_len << "\n"
              << "Connection >> " << this->conn_status << "\n"
              << "Content-Type >> " << this->content_type << "\n"
              << "CGI boolean >> " << this->bool_cgi << "\n"
              << "Body >> " << this->body << "\n"
              << "CGI env >> \n" << this->cgi_env << "\n"
              << "File upload filename >> " << this->filename << "\n"
              << "Boolean binary >> " << this->bool_binary << "\n"
              << "Binary content >> " << this->binary_data
              << std::endl;
    // bool_binary = false;
}

//-------------------- fetch the correct server scope from webserv ----------------
void Request::fetchServerScope(){

}

