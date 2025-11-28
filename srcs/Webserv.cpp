#include "Webserv.hpp"
#include "Server.hpp"

Webserv::Webserv(){
}

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
	// int epoll_fd = epoll_create(10);
	// std::cout << epoll_fd << std::endl;

	//if successful returns 0
	// epoll_ctl(epoll_fd, EPOLL_CTL_ADD, )
}


//check if the {} is even for the server and for the location scopes
//only the validation of the scopes should happen here no value parsing
int Webserv::scopeValidation(std::ifstream &file){
	std::string line, tok1, tok2;
	bool location_scope = false;
	bool server_scope = false;
	t_location location;

	int start_location = 0, end_location = 0;
	int start_server = 0, end_server = 0;

	/*------------------------inside the server scope------------------*/

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
		}
		if(location_scope && line.find("}") != std::string::npos){ //after reading the location scope
			location_scope = false;
			//need to add the values of the whole location scope into the map
			end_location++;
		}
		else if(location_scope == false && line.find("}") != std::string::npos){ //look for the end of the server scope
			std::stringstream ss(line);
			std::string tok1;
			ss >> tok1;

			// std::cout << "token of ending server scope >> " << tok1 << std::endl;
			if(tok1 == "}") end_server++;
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
	//when the server constructor has been successfully created, the server's whole scope from the config
	//file will be passed into the servers
	std::ifstream inputFile;
	inputFile.open(av);

	//validation first for the whole config file
	//get the pointer and while moving, parse the file into the server's attributes
	//then push that server back to the collection of servers in the webserv

	/* //this is not validation, this is parsing!!!
	while(reading until the end of the config file)
		if(server keyword found){
			- Server(pass reference of the file from the ifstream) -> this will create the object
			- from under the object, can do the value parsing
				-read line by line until the end of the server scope }
					-quit the reading when the server scope ends
		}
	-- since we r using ifstream, when we quit as we found the }, it will just automatically update
	the location of that place
	*/

	
	std::string	word;
	std::string	config_file;
	if(av) config_file = av;
	else config_file = "def.conf";

	std::cout << "config file >> "<< config_file << std::endl;
	std::ifstream file(config_file.c_str());
	// std::ifstream file2 = file;
	std::string line;

	if(scopeValidation(file) == 0) throw ConfigFileError();
		file.clear();
		file.seekg(0);
		
		while(getline(file, line)){ //read the whole file line by line
			std::cout << "line >> " << line << std::endl;
			if(line.find("server") != std::string::npos){
				std::string tok;
				std::stringstream ss(line);
				int server_scope = 1;
				
				while(ss >> tok){
					if(tok == "server") {
						server_scope++;
						servers.push_back(Server(file, server_scope)); //ther first scope of the server will be done
					}
					else break;
				}
			}
		}
}

//print each of the servers frin the webserv
void Webserv::printServers() const {
    std::cout << "\n========== PRINTING ALL SERVERS ==========\n" << servers.size() << std::endl;
    for (size_t i = 0; i < servers.size(); i++) {
        std::cout << "\nSERVER #" << i + 1 << ":\n";
        servers[i].print();
    }
}


ConfigValidationError::ConfigValidationError()
	: std::runtime_error("Error in config file") {}

//print location map
// void Webserv::print_location_map(){
// 	for(std::map<std::string, t_location>::iterator it = this->location_map.begin(); it != this->location_map.end(); ++it){
// 		std::cout << " " << it->first << " " 
// 		<< it->second.autoindex << " " 
// 		<< it->second.get << " " 
// 		<< it->second.post << " " 
// 		<< it->second.del << " " 
// 		<< it->second.upload_dir << " "
// 		<< it->second.root << std::endl;
// 	}
// }




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

