#include "Server.hpp"
#include "Connection.hpp"

Server::Server() :
	Pollable(-1), 
	_name(""), 
	_ip(""), 
	_port(""), 
	_root(""), 
	_max_size(0),
	_r_status(-1),
	_r_url("")
{}

Server::Server(const Server &other) :
	Pollable(other._fd),
	_name(other._name),
	_ip(other._ip),
	_port(other._port),
	_root(other._root),
	_max_size(other._max_size),
	_r_status(other._r_status),
	_r_url(other._r_url),
	_err_pages(other._err_pages),
	_locations(other._locations)
{
    (void)other;
}

Server& Server:: operator=(const Server &other)
{
    if (this != &other)
	{
		_fd = other._fd;
		_name = other._name;
		_ip = other._ip;
		_port = other._port;
		_root = other._root;
		_max_size = other._max_size;
		_r_status = other._r_status;
		_r_url = other._r_url;
		_err_pages = other._err_pages;
		_locations = other._locations;
	}
    return (*this);
}

Server::~Server() {}

// std::string Server::
// trimSemiColon(std::string val){
// 	if(val.find(";") == std::string::npos)	throw ConfigFileError();
// 	return (val.substr(0, val.length() - 1));
// }



int Server::validateHTTPCode(int &code){
	// for(size_t i = 0; i < val.length(); i++){
	// 	if(!isdigit(val[i]))	return 0; //not digit
	// }
	// if(val.length() > 3 || val.length() < 3) return 0;
	return (code >= 100 && code <= 599);
}

int	Server::parse_err_pages(std::stringstream &ss, std::map<int,std::string> &err_pg_container)
{
	std::vector<std::string> temp;
	std::string	token;
	while (ss >> token)
		temp.push_back(token);
	if (temp.size() <= 1)
		return (0);
	std::string	err_path = trimSemiColon(temp[temp.size() - 1]);
	for (int i = 0; i < temp.size() - 1; ++i)
	{
		int	key = std::atoi(temp[i].c_str());
		if (!validateHTTPCode(key))
			return (0);
		err_pg_container[key] = err_path;
	}
	return (1);
}

std::string	Server::trimSemiColon(std::string val)
{
	if(val.find(";") == std::string::npos)
		throw Error(val);
	return (val.substr(0, val.length() - 1));
}

int	Server::validateHTTPCode(int &code)
{
	return (code >= 100 && code <= 599);
}

int Server::inputData(std::string &line)
{
	std::stringstream	ss(line);
	std::string			token, value;

	ss >> token;
	if(token == "server_name") {
		if(!(ss >> value))
			return 0;
		else
			this->_name = trimSemiColon(value);
	}
	else if(token == "listen") {
		if(!(ss >> value)) return 0;
		else{
			int idx = value.find(":");
			this->_ip  = value.substr(0, idx);
			this->_port = trimSemiColon(value.substr(idx + 1));
		}
	}
	else if(token == "root"){
		if(!(ss >> value))
			return 0;
		else
			this->_root = trimSemiColon(value);
	}
	else if(token == "max_body_size"){
		if(!(ss >> value)) return 0;
		else{
			this->_max_size = atoi(value.c_str());
		}
	}
	else if (token == "return" && this->_r_status == 0)
	{
		if (!parse_return(ss, _r_status, _r_url))
			return (0);		
	}
	else if(token == "error_page")
	{
		parse_err_pages(ss, _err_pages);
	}
	return 1;	
}

int Server::inputLocation(std::string line, t_location &location)
{
	std::string token, val;
	std::stringstream ss(line);
	// if(line.find(";") == std::string::npos) throw ConfigFileError();

	ss >> token;
		// std::cout << "token  in inputLocation>> " << token << std::endl;
	if(token == "autoindex")
	{
		ss >> val;
		val = trimSemiColon(val);
		if(val == "on") location.autoindex = true;
		else location.autoindex = false;
	}
	else if(token == "methods")
	{
		while(ss >> val)
		{
			if (val.find(";") != std::string::npos)
				val = trimSemiColon(val);
			location.methods |= identify_method(val);
		}
		// std::cout << "methods >> " << method1 << " " << method2 << " " << method3 << std::endl;	
	}
	else if(token == "root"){
		ss >> val;
		location.root = trimSemiColon(val);
		// std::cout <<"location _root >> " << location._root << std::endl;
	}
	else if(token == "upload_dir"){
		ss >> val;
		location.upload_dir = trimSemiColon(val);
	}
	else if(token == "index"){
		while(ss >> val)
			location.index_files.push_back(trimSemiColon(val));
		// std::cout << location.index_files[0] << " " << location.index_files[1] << std::endl;
	}
	else if(token == "cgi"){
		std::string key;
		ss >> key >> val;
		location.cgi.insert(std::pair<std::string, std::string>(key, trimSemiColon(val)));
	}
	else if(token == "return" && location.r_status == 0)
	{
		if (!parse_return(ss, location.r_status, location.r_url))
			return (0);
	}
	else if (token == "proxy_pass")
	{
		ss >> val;
		location.rproxy = trimSemiColon(val);
	}
	else if(token == "error_page")
		parse_err_pages(ss, location.err_pages);
	return (1);
}

void	Server::print() const
{
    std::cout << "==================== SERVER ====================\n";
    std::cout << "Server Name: " << _name << "\n";
	std::cout << "Root: " << _root << "\n";
    std::cout << "IP: " << _ip << "\n";
    std::cout << "Port: " << _port << "\n";
    std::cout << "Max Body Size: " << _max_size << "\n";

    std::cout << "\n-- Error Pages --\n";
    for (std::map<int, std::string>::const_iterator it = _err_pages.begin(); it != _err_pages.end(); ++it) {
        std::cout << it->first << " => " << it->second << "\n";
    }

    std::cout << "\n-- Return Pages --\n";
	if (_r_status)
		std::cout << _r_status << (_r_url.empty() ? "" : " => ") << _r_url << std::endl;

    std::cout << "\n-- Locations --\n";
    for (std::map<std::string, t_location>::const_iterator it = _locations.begin(); it != _locations.end(); ++it) {
        const std::string &path = it->first;
        const t_location &loc = it->second;

        std::cout << "Location: " << path << "\n";
        std::cout << "  autoindex: " << (loc.autoindex ? "on" : "off") << "\n";
        std::cout << "  methods: "
                  << (loc.methods & GET ? "GET " : "")
                  << (loc.methods & POST ? "POST " : "")
                  << (loc.methods & DELETE ? "DELETE " : "") << "\n";
        std::cout << "  root: " << loc.root << "\n";
        std::cout << "  upload_dir: " << loc.upload_dir << "\n"
				  << "  proxy_pass: " << loc.rproxy << "\n";
        std::cout << "  index files: ";
        for (size_t i = 0; i < loc.index_files.size(); i++)
            std::cout << loc.index_files[i] << " ";
        std::cout << "\n";

        std::cout << "  CGI: ";
        for (std::map<std::string, std::string>::const_iterator ci = loc.cgi.begin(); ci != loc.cgi.end(); ++ci)
            std::cout << ci->first << " => " << ci->second << " ";
        std::cout << "\n";

        std::cout << "  Return pages: ";
			std::cout << loc.r_status << " " << loc.r_url << std::endl;
		std::cout << "  Error pages: ";
		print_map(loc.err_pages);
		std::cout << "\n\n";

    }
    std::cout << "=================================================\n";
}

int	Server::start()
{
	socklen_t	opt = 1;
	struct sockaddr_in	sock_addr;

	std::memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;	
	sock_addr.sin_port = htons(std::atoi(_port.c_str()));
	sock_addr.sin_addr.s_addr = inet_addr(_ip.c_str());
	if ((_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ||
		setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0 ||
		bind(_fd, (sockaddr *)&sock_addr, sizeof(sock_addr)) < 0 ||
		fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL, 0) | O_NONBLOCK) < 0 ||
		listen(_fd, SOMAXCONN) < 0)
	{
		int status = fail("Server: " + std::string(*this), errno);
		if (_fd > 0)
			close(_fd);
		_fd = -1;
		return (status);
	}
	std::cout << "[server]\t" << std::string(*this) << "\t| socket:" << _fd << " started - http://" << _ip << ":" << _port << std::endl;
	return (0);
}

Server::operator	fd() const
{
	return (_fd);
}

Server::operator	int() const
{
	return (std::atoi(_port.c_str()));
}

Server::operator	std::string() const
{
	return (_ip + ":" + _port + ":" + _name);
}

std::string	Server::name() const
{
	return (_name);
}

std::string	Server::ip() const
{
	return (_ip);
}

std::string	Server::port() const
{
	return (_port);
}


std::string	Server::root() const
{
	return (_root);
}

int	Server::r_status() const
{
	return (_r_status);
}

std::string	Server::r_url() const
{
	return (_r_url);
}

const std::map<std::string, t_location>&	Server::locations() const
{
	return (_locations);
}

void	Server::handle(uint32_t events)
{
		Connection	*con = new Connection(this);
		fd	con_fd = *con;
		// std::cout << "client fd " << con_fd << std::endl;
		if (con_fd < 0)
		{
			delete con;
			fail("Server: Client", errno);
		}
		if (Epoll::instance().add_ptr(con, EPOLLIN) < 0)
		{
			std::cout << "why here" << std::endl;
			fail("Epoll: Client", errno);
			delete con;
		}
}
