#include "Request.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <string>

std::string cgi_env = "";

Request::Request():
    _method(""),
    _path(""),
    version(""),
    hostname(""),
    port(0),
    content_type(-1),
    content_len(0),
    conn_status(""),
    _query(""),
    _body(""),
    _category(NORMAL),
    bool_cgi(false),
    bool_boundary(false),
    bool_referer(false),
    bool_binary(false),

    cgi_env(""),
    boundary(""),
    referer("")
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

void Request::handleChunkedBody(const char* raw_body)
{
    const char* ptr = raw_body;
    _body.clear();

    while (true)
    {
        // 1. read chunk size
        const char* line_end = strstr(ptr, "\r\n");
        if (!line_end) break; // malformed
        
        std::string size_str(ptr, line_end - ptr);
        size_t chunk_size = 0;
        std::stringstream ss;
        ss << std::hex << size_str;
        ss >> chunk_size;

        ptr = line_end + 2; // move past \r\n

        if (chunk_size == 0) break;

        // 2. append chunk data
        _body.append(ptr, chunk_size);

        ptr += chunk_size + 2; // move past chunk + trailing \r\n
    }
}

void Request::extractMultipartFiles(const std::string &body)
{
    std::string delimiter = "--" + boundary;
    std::string close_delimiter = delimiter + "--";

    size_t pos = 0;

    while (true)
    {
        size_t start = body.find(delimiter, pos);
        if (start == std::string::npos)
            break;

        start += delimiter.length();

        // End marker
        if (body.compare(start, 2, "--") == 0)
            break;

        // Skip CRLF
        if (body.compare(start, 2, "\r\n") == 0)
            start += 2;

        size_t header_end = body.find("\r\n\r\n", start);
        if (header_end == std::string::npos)
            break;

        std::string headers = body.substr(start, header_end - start);

        size_t data_start = header_end + 4;
        size_t data_end = body.find("\r\n" + delimiter, data_start);
        if (data_end == std::string::npos)
            break;

        std::string binary = body.substr(data_start, data_end - data_start);

        parseSinglePart(headers, binary);

        pos = data_end;
    }
}
void Request::parseSinglePart(const std::string &headers,
                              const std::string &binary)
{
    binary_file file;

    std::istringstream iss(headers);
    std::string line;

    while (std::getline(iss, line))
    {
        if (line.find("Content-Disposition") != std::string::npos)
        {
            size_t fn = line.find("filename=\"");
            if (fn != std::string::npos)
            {
                fn += 10;
                size_t end = line.find("\"", fn);
                file.filename = line.substr(fn, end - fn);
            }
        }
        else if (line.find("Content-Type") != std::string::npos)
        {
            size_t pos = line.find(":");
            file.content_type = line.substr(pos + 1);
            file.content_type.erase(0, file.content_type.find_first_not_of(" "));
        }
    }

    file.data = binary;
    _upload_files.push_back(file);
}


//----------------- request parsing --------------------
void Request::parseRequest(const char *raw_request){
    request_cat request_category;
    
    bool_cgi = false;
    bool hostname = false;
    bool bool_content_len = false;
    bool bool_connection = false;
    bool bool_cont_type = false;
    bool bool_main_headers = false;
    bool bool_transfer = false;

    size_t idx2;
    int length;
    std::string line;

    /*-------------------------header handler  ----------------------*/
    const char* body_start = strstr(raw_request, "\r\n\r\n"); //returns the pointer to the first \r\n\r\n
    //will only process the header's 

    if (!body_start) return;
    size_t header_len = body_start - raw_request;
    std::string header(raw_request, header_len); //the main header of the request
    std::istringstream iss(header); //take hte request as a string stream

    //identify the parsing type by getting the headers first
    //process each header line
    while(std::getline(iss, line)){
        std::stringstream stream(line);
        std::string token;
        while(stream >> token){ //toLower is applied because headers are case-insensitive
            // std::cout << token << std::endl;
            if(token == "POST" || token == "GET"  || token == "DELETE") this->_method = token;
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
                if(this->_method == "GET" && queryIdx != std::string::npos) //query parsing
                    this->_query = token.substr(queryIdx + 1, token.length() - queryIdx);
                this->_path = token;
                
            }
            else if(token == "HTTP/1.1") this->version = token;
            else if(toLower(token)  == "host:") hostname = true;
            else if(toLower(token)  == "content-length:") bool_content_len = true;
            else if(toLower(token)  == "connection:") bool_connection = true;
            else if(!bool_boundary && toLower(token) == "content-type:") bool_cont_type = true;
            else if (toLower(token) == "transfer-encoding:") bool_transfer = true;
            else if(toLower(token) == "referer:"){ 
                bool_referer = true;
                request_category = REDIRECTION;
            }
            else if(bool_transfer){
                if(token == "chunked") this->bool_chunked = true;
            } 
            else if(bool_content_len){
                if(validate_len(token)) this->content_len = atoi(token.c_str());
                cgi_env += "CONTENT_LENGTH=" + token + "\n";
                bool_content_len = false;
            }
            else if(hostname){
                //will be the value after the host
                int idx = token.find(":"); //index of the ':'
                this->hostname = token.substr(0, idx);
                token = token.substr(idx + 1, (token.length()) - idx - 1).c_str();
                this->port = atoi(token.c_str());
                cgi_env += "SERVER_PORT=" + token + "\n";
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
                cgi_env += "CONTENT_TYPE=" + token + "\n";
                bool_cont_type = false;
            }
            else if(token.find("boundary=") != std::string::npos){
                int idx = token.find("=");
                this->boundary = token.substr(idx + 1, token.length() - idx - 1);
                bool_boundary = true;
                request_category = UPLOAD;
                std::cout << "boundary >> " << this->boundary << std::endl;
            }
            else if(bool_referer){
                this->referer = token;
                bool_referer = false;
            }
        }
    }
    /*-------------------------body handler  ----------------------*/

    
    // size_t body_len = strlen(raw_request) - header_len;
    body_start += 4;  //the real starting of the body context

     //normal and unchunked parsing (without the transfer-encoding: chunked)
    if(!bool_chunked){         
        _body.assign(body_start, this->content_len);
        if(bool_boundary)  extractMultipartFiles(_body);
    }
    else                                        //chunked parsing
       { handleChunkedBody(body_start);
        std::cout << "handling of chunk working" << std::endl;
    }

    std::cout << "body print out >> " << _body << std::endl;

    if(this->bool_boundary){
        // extractMultipartFile(body);
        // printUploadedFiles();
        std::cout << "multipart size" << "\n"
                    << _body.size() << std::endl;
    }

    if(bool_cgi){
        cgi_env += "REQUEST_METHOD=" + this->_method + "\n" +
                    "QUERY_STRING=" + this->_query + "\n" +
                    "SERVER_NAME=" + this->hostname + "\n" +
                    "SERVER_PROTOCOL=" + this->version + "\n" +
                    "SCRIPT_NAME=" + this->_path + "\n";
    }
}

//-------------------- fetch the correct server scope from webserv 


void Request::printUploadedFiles() const
{
    std::cout << "=== Uploaded Files (" 
              << this->_upload_files.size() 
              << ") ===\n";

    for (size_t i = 0; i < this->_upload_files.size(); ++i)
    {
        const binary_file &f = this->_upload_files[i];

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

void	Request::set_category(request_cat type)
{
	_category = type;
}

std::string Request::path() const
{
    return (_path);
}

std::string Request::method() const
{
    return (_method);
}

request_cat Request::category() const
{
    return (_category);
}

std::vector<binary_file>    Request::binary_data() const
{
    return (_upload_files);
}
