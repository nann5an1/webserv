#include "Webserv.hpp"

Webserv::Webserv(){}

Webserv::~Webserv(){}
Webserv::Webserv(const Webserv &other) {
	(void)other;
}

Webserv& Webserv:: operator=(const Webserv &other) {
	(void)other;
	return *this;
}

void Webserv::watchServer(){
	// while(epoll())
	int epoll_fd = epoll_create(10);
	std::cout << epoll_fd << std::endl;

	//if successful returns 0
	// epoll_ctl(epoll_fd, EPOLL_CTL_ADD, )
}

std::string Webserv::trimSemiColon(std::string val){
	if(val.find(";") == std::string::npos)	throw ConfigFileError();
	return (val.substr(0, val.length() - 1));
}

int Webserv::validateHTTPCode(std::string &val){
	for(size_t i = 0; i < val.length(); i++){
		if(!isdigit(val[i]))	return 0; //not digit
	}
	if(val.length() > 3 || val.length() < 3) return 0;
	return 1;
}
//get the data from the config file by line iteration
int Webserv::inputData(std::string line){
	std::string token, value;  
	std::stringstream ss(line);
	ss >> token >> value;

	if(token == "server_name")	this->server_name = value;
	else if(token == "listen"){
		int idx = value.find(":");
		this->listen_ip  = value.substr(0, idx);
		this->listen_port = value.substr(idx + 1, value.length() - idx - 1);
	}
	else if(token == "error_page"){
		std::string path;
		ss >> path;
		// std::cout << "path >> " << path << std::endl;
		path = trimSemiColon(path);
		if(!validateHTTPCode(value)) return 0;
		this->err_pages.insert(std::pair<int, std::string>(atoi(value.c_str()), path));
	}
	else if(token == "client_max_body_size")	this->max_body_size = atoi(value.c_str());

	return 1;	
}

int Webserv::inputLocation(std::string line, t_location &location){
	std::string token;
	
	std::stringstream ss(line);
	ss >> token;

	if(token == "autoindex"){
		std::string val;
		ss >> val;
		val = trimSemiColon(val);
		if(val == "on") location.autoindex = true;
	}
	else if(token == "allow_methods"){
		std::string method1, method2, method3;
		ss >> method1 >> method2 >> method3;
		// std::cout << "methods >> " << method1 << " " << method2 << " " << method3 << std::endl;

		if(method1.find(";") != std::string::npos)	method1 = trimSemiColon(method1);
		if(method2.find(";") != std::string::npos)	method2 = trimSemiColon(method2);
		if(method3.find(";") != std::string::npos)	method3 = trimSemiColon(method3);

		if(method1 == "GET")	location.get = true;
		if(method2 == "POST")	location.post = true;
		if(method3 == "DELETE")	location.del = true;
	}
	else if(token == "root"){
		std::string val;
		ss >> val;
		val = trimSemiColon(val);
		location.root = val;
	}
	else if(token == "upload_dir"){
		std::string val;
		ss >> val;
		val = trimSemiColon(val);
		location.upload_dir = val;
	}
	else if(token == "index"){
		std::string val1, val2;
		ss >> val1 >> val2;
		location.index_files.push_back(val1);
		location.index_files.push_back(val2);
		// std::cout << location.index_files[0] << " " << location.index_files[1] << std::endl;
	}
	else if(token == "cgi"){
		std::string key, val;
		ss >> key >> val;
		val = trimSemiColon(val);
		location.cgi.insert(std::pair<std::string, std::string>(key, val));
	}
	else if(token == "return"){
		std::string key, val;
		ss >> key >> val;
		val = trimSemiColon(val);
		if(!validateHTTPCode(key)) return 0;
		location.ret_pages.insert(std::pair<int, std::string>(atoi(key.c_str()), val));
	}
	return 1;
}


//check if the {} is even for the server and for the location scopes
int Webserv::scopeValidation(std::ifstream &file){
	std::string line, tok1, tok2;
	bool location_scope = false;
	bool server_scope = false;
	t_location location;

	int start_location = 0, end_location = 0;
	int start_server = 0, end_server = 0;

	/*------------------------inside the server scope----------------*/

	//if we are in the server scope now read the line until the ending of the server's }
	while(getline(file, line)){
		// std::cout << "line check >> " << line << std::endl;
		std::string token;
		std::stringstream ss(line);
		ss >> token;
		if(token == "server"){
			std::stringstream ss(line);

			ss >> tok1 >> tok2;
			// std::cout << "line " << line << std::endl;
			// std::cout << "token " << token << std::endl;
			// std::cout << "tok2 " << tok2 << std::endl;

			//handle the { variations
			if(tok2 == " "){
				std::string tok;
				getline(file, line);
				std::stringstream ss(line);
				ss >> tok;
				if(tok != "{")	return 0;
			}
			server_scope = true;
			start_server++;
		}
		else if(line.find("location") != std::string::npos){
			std::stringstream ss(line);
			std::string tok1, tok2, tok3;
			ss >> tok1 >> tok2 >> tok3;

			// std::cout << "token1 location >> " << tok1 << std::endl;
			// std::cout << "token2 location >> " << tok2 << std::endl;
			// std::cout << "token3 location >> " << tok3 << std::endl;
			
			if(tok1 != "location") return 0;
			if(tok3 != "{"){ //if the opening isn't on the same line	
				getline(file, line); //read the next line if the opening isn't on the same line
				// std::cout << "Moved to another line of location >> " << line << std::endl;
				std::stringstream ss(line);
				ss >> tok1;
				if(tok1 != "{") return 0;
				else{
					location_scope = true;
					start_location++;
				}
			}
			else{
				location_scope = true;
				start_location++;
			}
			location = t_location();
			this->location_path = tok2;
		}
		if(location_scope && line.find("}") != std::string::npos){ //after reading the location scope
			location_scope = false;
			//need to add the values of the whole location scope into the map
			this->location_map.insert(std::pair<std::string, t_location>(this->location_path, location));
			end_location++;
		}
		else if(location_scope == false && line.find("}") != std::string::npos){ //look for the end of the server scope
			std::stringstream ss(line);
			std::string tok1;
			ss >> tok1;

			// std::cout << "token of ending server scope >> " << tok1 << std::endl;
			if(tok1 == "}") end_server++;
		}
		if(server_scope && location_scope == false){	//inputting the data from the server scope
			if(!inputData(line)) return 0;
			
		}
		else if(server_scope && location_scope){	//inputting the data from the location scope
			if(!inputLocation(line, location)) return 0;

			// std::cout << "get: " << location.get << std::endl;
			// std::cout << "post: " << location.post << std::endl;
			// std::cout << "del: " << location.del << std::endl;

		}
	}
	// std::cout << "start server_count " << start_server << std::endl;
	// std::cout << "end server_count " << end_server << std::endl;

	// std::cout << "start location_count " << start_location << std::endl;
	// std::cout << "end location_count " << end_location << std::endl;

	
	//return 0 if the number of server and location scopes are not even
	if(start_server != end_server || start_location != end_location) return 0;
	return 1;
}

void Webserv::fileParser(char *av){
	std::string	word;
	std::string	config_file;
	if(av) config_file = av;
	else config_file = "def.conf";
	// bool server_scope = false;

	std::ifstream file(config_file.c_str());
	std::string line;

	if(scopeValidation(file)){
		// server_scope = true;
		std::cout << "valid config file" << std::endl;
		std::cout << "/nserver_name: " << this->server_name << std::endl;
		std::cout << "listen_ip: " << this->listen_ip << std::endl;
		std::cout << "listen_port: " << this->listen_port << std::endl;
		std::cout << "max_body_size: " << this->max_body_size << std::endl;
	
		printMap(this->err_pages);
		
		// printMap(this->location_map);
	}
	else{
		throw ConfigFileError();
	}
	// std::cout << "server_scope status -> " << server_scope << std::endl;
}


//print location map
void Webserv::print_location_map(){
	for(std::map<std::string, t_location>::iterator it = this->location_map.begin(); it != this->location_map.end(); ++it){
		std::cout << " " << it->first << " " 
		<< it->second.autoindex << " " 
		<< it->second.get << " " 
		<< it->second.post << " " 
		<< it->second.del << " " 
		<< it->second.upload_dir << " "
		<< it->second.root << std::endl;
	}
}



ConfigFileError::ConfigFileError()
	: std::runtime_error("Error in config file") {}



//trim spaces/tabs and validate
// std::string Webserv::trimSpaces(std::string line){
// 	std::string serverHeadline = "";
	
// 	// std::cout << line.length() << std::endl;

// 	for(size_t i = 0; i < line.length(); i++){
// 		if(serverHeadline != "server" && line[i] == ' ' && serverHeadline.length() != 0) break;
// 		if(serverHeadline.length() == 0 && (line[i] == ' ' || line[i] == '\t')) continue;
// 		if(line[i] != ' ')
// 			serverHeadline += line[i];
// 	}
// 	// std::cout << "after spaces trimmed >> " << serverHeadline << std::endl;
// 	if(serverHeadline.length() == 7 && serverHeadline == "server{") return serverHeadline;
// 	else if(serverHeadline.length() == 6 && serverHeadline == "server") return serverHeadline;
// 	else if(serverHeadline.length() == 1 && serverHeadline == "{") return serverHeadline;
// 	else return "";
		
// 	return (serverHeadline);
// }

//config file parser
// Webserv::fileParse(char *av){
	
// }

