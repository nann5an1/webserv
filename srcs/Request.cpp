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

// void Request::parseSinglePart(const std::string &headers,
//                               const std::string &binary)
// {
//     binary_file filePart;

//     // --- Extract filename="..." ---
//     std::string filename_key = "filename=\"";
//     size_t filename_pos = headers.find(filename_key);
//     if (filename_pos != std::string::npos) {
//         filename_pos += filename_key.size();
//         size_t end = headers.find("\"", filename_pos);
//         if (end != std::string::npos)
//             filePart.filename = headers.substr(filename_pos, end - filename_pos);
//     }

//     // --- Extract Content-Type: ---
//     std::string ct_key = "Content-Type:";
//     size_t ct_pos = headers.find(ct_key);
//     if (ct_pos != std::string::npos) {
//         ct_pos += ct_key.size();

//         // Skip spaces after Content-Type:
//         while (ct_pos < headers.size() && (headers[ct_pos] == ' ' || headers[ct_pos] == '\t'))
//             ct_pos++;

//         // End at newline
//         size_t end = headers.find("\r\n", ct_pos);
//         if (end == std::string::npos)
//             end = headers.size();

//         filePart.content_type = headers.substr(ct_pos, end - ct_pos);
//     }

//     // --- Store binary data ---
//     filePart.data = binary;

//     // Save to request
//     this->upload_files.push_back(filePart);
// }


// void Request::extractMultipartFile(const std::string &body) {
//     std::string boundary = "--" + this->boundary;
//     size_t pos = 0;

//     while (true) {
//         // 1. Find boundary
//         size_t boundary_pos = body.find(boundary, pos);
//         if (boundary_pos == std::string::npos)
//             break;

//         // Move to after boundary line
//         size_t headers_start = body.find("\r\n", boundary_pos);
//         if (headers_start == std::string::npos)
//             break;
//         headers_start += 2;

//         // 2. Find header/body separator
//         size_t data_start = body.find("\r\n\r\n", headers_start);
//         if (data_start == std::string::npos)
//             break;
//         data_start += 4; // skip the empty line

//         // 3. Find next boundary FROM data_start
//         size_t next_boundary = body.find(boundary, data_start);
//         if (next_boundary == std::string::npos)
//             break; // last part

//         // 4. Extract headers and binary safely
//         std::string headers = body.substr(headers_start,
//                                           (data_start - 4) - headers_start);

//         std::string binary = body.substr(data_start,
//                                          next_boundary - data_start);

//         parseSinglePart(headers, binary);

//         pos = next_boundary;
//     }
// }



//----------------- request parsing --------------------
void Request::parseRequest(const char *raw_request){
    // std::map<std::string, content_category> content_types;
    request_cat request_category;
    
    bool_cgi = false;
    bool hostname = false;
    bool bool_content_len = false;
    bool bool_connection = false;
    bool bool_cont_type = false;
    bool bool_main_headers = false;

    size_t idx2;
    int length;
    std::string line;
    
    const char* body_start = strstr(raw_request, "\r\n\r\n"); //returns the pointer to the first \r\n\r\n
    size_t header_len = body_start - raw_request;
    size_t body_len = strlen(raw_request) - header_len;

    std::string header(raw_request, header_len);
    std::string body(body_start + 4, body_len); //binary safe since the remaining data is dumped into body


    /*-------------------------header handler  ----------------------*/
    //will only process the header's 
    std::istringstream iss(header); //take hte request as a string stream

    //process each header line
    while(std::getline(iss, line)){
        std::stringstream stream(line);
        std::string token;
        
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
                    if(ext == "php" || ext == "py" || ext == "pl" || ext == "sh" || ext == "rb") {
                        this->bool_cgi = true;
                        request_category = CGI;
                    }
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
            else if(!bool_boundary && toLower(token) == "content-type:") bool_cont_type = true;
            else if(toLower(token) == "referer:"){ 
                bool_referer = true;
                request_category = REDIRECTION;
            }
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
                bool_boundary = true;
                request_category = UPLOAD;
                std::cout << "boundary >> " << this->boundary << std::endl;
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
        }
    }
    /*-------------------------body handler  ----------------------*/
    if(this->bool_boundary){
        // extractMultipartFile(body);
        // printUploadedFiles();
        std::cout << "multipart size" << "\n"
                    << body.size() << std::endl;
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
              << "\n << CGI env >> \n" << this->cgi_env << "\n"
              << "File upload filename >> " << this->filename << "\n"
              << "Boolean boundary >> " << this->bool_boundary << "\n"
            //   << "Binary content >> " << this->binary_data
              << std::endl;
}

//-------------------- fetch the correct server scope from webserv 
void Request::fetchServerScope(){

}

void Request::printUploadedFiles() const
{
    std::cout << "=== Uploaded Files (" 
              << this->upload_files.size() 
              << ") ===\n";

    for (size_t i = 0; i < this->upload_files.size(); ++i)
    {
        const binary_file &f = this->upload_files[i];

        std::cout << "\n[File #" << i+1 << "]\n";
        std::cout << "Filename    : " << f.filename << "\n";
        std::cout << "Content-Type: " << f.content_type << "\n";
        std::cout << "Binary Size : " << f.data.size() << " bytes\n";

        // Optional: show first 16 bytes of binary content
        std::cout << "Binary Preview (hex): ";
        for (size_t j = 0; j < f.data.size() && j < 16; ++j) {
            unsigned char byte = (unsigned char)f.data[j];
            std::cout << std::hex;
            if (byte < 16) std::cout << '0';
            std::cout << (int)byte << " ";
            std::cout << std::dec;
        }
        std::cout << "\n";
    }
}
