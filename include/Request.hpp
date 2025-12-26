#ifndef REQUEST_HPP
#define REQUEST_HPP

// #include "Webserv.hpp"
// #include "Server.hpp"
#include <iostream>
#include <map>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <bits/stdc++.h>

#include "Utils.hpp"

enum content_category{
	ERROR,
	JSON,
	URLENCODED,
	FORM
};

enum request_cat
{
	NORMAL,
	CGI,
	REDIRECTION,
	FILE,
};

struct binary_file{
	std::string filename;
	std::string content_type;
	std::string data; 
};

class Request{
	private:
		std::string _method;
		std::string _path;
		std::string version;
		std::string hostname;
		int port;
		int content_type;        	// Data Content Type
		int content_len;
		std::string conn_status;	// Keep Alive or Close
		std::string _query;
		std::string _body;
		request_cat	_category;		// Request Category : CGI, Upload, etc.
		bool bool_cgi;
		bool bool_boundary;
		bool bool_referer;
		bool bool_binary;
		bool bool_chunked;
		
		std::string binary_data; //data from the extractMultipleParts
		std::vector<std::string> _cgi_env;
		std::string boundary;
		std::string referer;
		std::map<std::string, content_category> content_types;
		std::vector<binary_file> _upload_files;

	public:
		Request();
		~Request();
		// Request(const Request &other);
		// Request &operator=(const Request &other);
		void parseRequest(const char *raw_request);
		void handleChunkedBody(const char* body);
		void extractMultipartFiles(const std::string &body);
		void parseSinglePart(const std::string &headers, const std::string &binary);
		void printUploadedFiles() const;
		
		void	set_category(request_cat type);

		std::string	path() const;
		std::string	method() const;
		std::string	body() const;
		request_cat	category() const;
		std::vector<std::string>	cgi_env() const;
		std::vector<binary_file>	upload_files() const;


		
};

#endif
