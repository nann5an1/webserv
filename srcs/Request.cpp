#include "Request.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>

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
	bool_binary(false),
	bool_chunked(false),
	_cgi_env(""),
	boundary(""),
	referer("")
{
	content_types["application/json"] = JSON;
	content_types["application/x-www-form-urlencoded"] = URLENCODED;
	content_types["multipart/form-data"] = FORM;
	// content_types["text/plain"] = NORMAL;
}

Request::~Request() {}

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

/* ========================= Parse handling the chunked body  =========================*/
void Request::handleChunkedBody(const char* raw_body)
{
	const char* ptr = raw_body;
	_body.clear();

	while (true)
	{
		// 1. read chunk size line
		const char* line_end = std::strstr(ptr, "\r\n");
		if (!line_end) break; // malformed
		
		std::string size_str(ptr, line_end - ptr);
		
		// Handle chunk extensions (e.g., "5;foo=bar")
		size_t semicolon = size_str.find(';');
		if (semicolon != std::string::npos) {
			size_str = size_str.substr(0, semicolon);
		}
		
		size_t chunk_size = 0;
		std::stringstream ss;
		ss << std::hex << size_str;
		ss >> chunk_size;

		ptr = line_end + 2; // move past \r\n after chunk size

		if (chunk_size == 0) break; // last chunk (0\r\n)

		// 2. append EXACTLY chunk_size bytes to body
		_body.append(ptr, chunk_size);
		ptr += chunk_size;
		
		// 3. Skip the mandatory \r\n after chunk data
		// According to HTTP/1.1 spec, each chunk MUST end with \r\n
		if (ptr[0] == '\r' && ptr[1] == '\n') {
			ptr += 2;
		}
	}
}

/* ========================= Extraction of multipart files =========================*/
void Request::extractMultipartFiles(const std::string &body)
{
	std::string delimiter = "--" + boundary;
	size_t pos = 0;

	std::cout << "=== Starting multipart extraction ===" << std::endl;
	std::cout << "Boundary: [" << boundary << "]" << std::endl;
	std::cout << "Body length: " << body.length() << std::endl;

	while (true)
	{
		size_t start = body.find(delimiter, pos);
		if (start == std::string::npos)
			break;

		start += delimiter.length();

		// Check for end marker "--"
		if (start + 2 <= body.length() && body.compare(start, 2, "--") == 0)
			break;

		// Skip CRLF or LF after boundary
		if (start < body.length() && body[start] == '\r' && start + 1 < body.length() && body[start + 1] == '\n')
			start += 2;
		else if (start < body.length() && body[start] == '\n')
			start += 1;

		// Find end of headers (blank line)
		size_t header_end = body.find("\r\n\r\n", start);
		if (header_end == std::string::npos) {
			// Try just \n\n    
			header_end = body.find("\n\n", start);
			if (header_end == std::string::npos)
				break;
			
			std::string headers = body.substr(start, header_end - start);
			size_t data_start = header_end + 2;
			size_t data_end = body.find("\n" + delimiter, data_start);
			if (data_end == std::string::npos)
				data_end = body.find(delimiter, data_start);
			if (data_end == std::string::npos)
				break;
			std::string file_data = body.substr(data_start, data_end - data_start);
			parseSinglePart(headers, file_data);
			pos = data_end;
		} else {
			std::string headers = body.substr(start, header_end - start);
			size_t data_start = header_end + 4;
			size_t data_end = body.find("\r\n" + delimiter, data_start);
			if (data_end == std::string::npos) {
				data_end = body.find("\n" + delimiter, data_start);
				if (data_end == std::string::npos)
					break;
			}
			
			std::string file_data = body.substr(data_start, data_end - data_start);
			parseSinglePart(headers, file_data);
			pos = data_end;
		}
	}
}

/* ========================= parse single part of the multipart =========================*/
void Request::parseSinglePart(const std::string &headers,
							  const std::string &binary)
{
	std::cout << "FUCKIG PARSE SINGLE IS CALLED" << std::endl;
	binary_file file;

	std::istringstream iss(headers);
	std::string line;

	while (std::getline(iss, line))
	{
		// Remove trailing \r if present
		if (!line.empty() && line[line.length()-1] == '\r')
			line.erase(line.length()-1);
			
		if (line.find("Content-Disposition") != std::string::npos)
		{
			size_t fn = line.find("filename=\"");
			if (fn != std::string::npos)
			{
				fn += 10;
				size_t end = line.find("\"", fn);
				file.filename = line.substr(fn, end - fn);
				std::cout << "<< DEBUG >> FILENAME -> " << file.filename << std::endl;
			}
		}
		else if (line.find("Content-Type") != std::string::npos)
		{
			size_t pos = line.find(":");
			if (pos != std::string::npos)
			{
				file.content_type = line.substr(pos + 1);
				// Trim leading whitespace
				file.content_type.erase(0, file.content_type.find_first_not_of(" \t"));
				// Trim trailing whitespace
				size_t end = file.content_type.find_last_not_of(" \t\r\n");
				if (end != std::string::npos)
					file.content_type = file.content_type.substr(0, end + 1);
			}
		}
	}
	file.data = binary;
	std::cout << "<< DEBUG >> FILE SIZE -> " << file.data.size() << std::endl;
	_upload_files.push_back(file);
	
	std::cout << "Added file: " << file.filename << " (" << file.data.size() << " bytes)" << std::endl;
}

/* ========================= Main request Parsing =========================*/
void Request::parseRequest(const char *raw_request){
	request_cat request_category = NORMAL;
	
	bool_cgi = false;
	bool_chunked = false;
	bool hostname = false;
	bool bool_content_len = false;
	bool bool_connection = false;
	bool bool_cont_type = false;
	bool bool_transfer = false;
    bool bool_file = false;

	size_t idx2;
	int length;
	std::string line;

	/*-------------------------header handler  ----------------------*/
	const char* body_start = std::strstr(raw_request, "\r\n\r\n");
	
	if (!body_start) return;
	size_t header_len = body_start - raw_request;
	std::string header(raw_request, header_len);
	std::istringstream iss(header);

	// Process each header line
	while(std::getline(iss, line)){
		// Remove trailing \r if present
		if (!line.empty() && line[line.length()-1] == '\r')
			line.erase(line.length()-1);
			
		std::stringstream stream(line);
		std::string token;
		while(stream >> token){

			if(token == "POST" || token == "GET"  || token == "DELETE") this->_method = token;
			else if(token[0] == '/'){
				size_t idx = token.find(".");       //search of extension
				size_t queryIdx = token.find("?"); //search for query
				
				if(idx != std::string::npos){ //consider if there's a '.' for extensions
					idx2 = token.find("?");
					if(idx2 == std::string::npos) {
						idx2 = 0;
						length = token.length() - idx - 1;
					}
					else length = idx2 - idx - 1;
					
					std::string ext = token.substr(idx + 1, length);
 					if((ext == "php" || ext == "py" || ext == "pl" || ext == "sh" || ext == "rb")) {
						this->bool_cgi = true;
						request_category = CGI;
					}
				}
                else if((this->_method == "POST" || this->_method == "DELETE") && idx == std::string::npos) //no extension and the token is "POST" -> upload directory 
                    bool_file = true;
				if(queryIdx != std::string::npos){
                    this->_query = token.substr(queryIdx + 1, token.length() - queryIdx);
                    size_t path_len = token.length() - this->_query.length();
                    this->_path = token.substr(0, path_len - 1);
                }
                else{
                    // std::cout << "? not found DEBUG >> " << token << std::endl;
                    this->_path = token; 
                }  
			}
			else if(token == "HTTP/1.1") this->version = token;
			else if(toLower(token) == "host:") hostname = true;
			else if(toLower(token) == "content-length:") bool_content_len = true;
			else if(toLower(token) == "connection:") bool_connection = true;
			else if(!bool_boundary && toLower(token) == "content-type:") bool_cont_type = true;
			else if(toLower(token) == "transfer-encoding:") bool_transfer = true;
			else if(bool_transfer){
				if(token == "chunked") this->bool_chunked = true;
				bool_transfer = false;
			} 
			else if(bool_content_len){
				if(validate_len(token)) this->content_len = atoi(token.c_str());
				_cgi_env += "CONTENT_LENGTH=" + token + "\n";
				bool_content_len = false;
			}
			else if(hostname){
				int idx = token.find(":");
				this->hostname = token.substr(0, idx);
				if(idx != (int)std::string::npos) {
					token = token.substr(idx + 1, (token.length()) - idx - 1).c_str();
					this->port = atoi(token.c_str());
					_cgi_env += "SERVER_PORT=" + token + "\n";
				}
				hostname = false;
			}
			else if(bool_connection){
				this->conn_status = token;
				bool_connection = false;
			}
            else if(bool_cont_type && bool_file)
                request_category = FILEHANDLE;
			else if(bool_cont_type){
				// Check if this token contains content type
				std::string lower_token = toLower(token);
				
				// Handle "multipart/form-data;" case
				if(lower_token.find(";") != std::string::npos) {
					lower_token = lower_token.substr(0, lower_token.find(";"));
				}
				
				if(content_types.find(lower_token) != content_types.end()) {
					this->content_type = content_types[lower_token];
					_cgi_env += "CONTENT_TYPE=" + lower_token + "\n";
				} else {
					std::cout << "Unknown content type: " << token << std::endl;
				}
				bool_cont_type = false;
			}
			if(token.find("boundary=") != std::string::npos){ //boundary does not help identify the file upload
				std::cout << "token" << token << std::endl;
				int idx = token.find("=");
				this->boundary = token.substr(idx + 1, token.length() - idx - 1);
				bool_boundary = true;
			}
		}
	}

	/*-------------------------body handler  ----------------------*/
	body_start += 4;  // Move to the actual body content

	if(bool_chunked) handleChunkedBody(body_start); // This populates _body with decoded content
	else _body.assign(body_start, this->content_len);

	//Parse multipart files if boundary is present
	if(bool_boundary && !_body.empty()) {
		extractMultipartFiles(_body);
	}
	if(bool_cgi)
	{
		std::ostringstream oss;

		oss << "REQUEST_METHOD=" << this->_method << "\n"
			<< "SERVER_NAME=" << this->hostname << "\n"
			<< "SERVER_PROTOCOL=" << this->version << "\n"
			<< "SCRIPT_NAME=" << this->_path.substr(0, _path.find("?")) << "\n"
			<< "QUERY_STRING=" << this->_query << "\n";
		_cgi_env += oss.str();
	}
	
	this->_category = request_category;
}


/*----------------------------Print Uploaded Files--------------------------------*/
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

		// Show first 64 bytes of content (or full content if shorter)
		std::cout << "Content Preview: ";
		size_t preview_len = (f.data.size() < 64) ? f.data.size() : 64;
		for (size_t j = 0; j < preview_len; ++j) {
			char c = f.data[j];
			if (c >= 32 && c <= 126) // printable ASCII
				std::cout << c;
			else if (c == '\n')
				std::cout << "\\n";
			else if (c == '\r')
				std::cout << "\\r";
			else
				std::cout << ".";
		}
		if (f.data.size() > 64)
			std::cout << "...";
		std::cout << "\n";
	}
}

void Request::set_category(request_cat type)
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

std::string Request::body() const
{
	return (_body);
}

std::string Request::cgi_env() const
{
	return (_cgi_env);
}

std::vector<binary_file> Request::upload_files() const
{
	return (_upload_files);
}