#include "Server.hpp"
#include "Connection.hpp"

Server::Server() :
	_fd(-1), 
	_name(""), 
	_ip(""), 
	_port(""), 
	_root(""), 
	_max_size(0),
	_r_status(0),
	_r_url("")
{}

Server::Server(const Server &other) :
	_fd(other._fd),
	_name(other._name),
	_ip(other._ip),
	_port(other._port),
	_root(other._root),
	_max_size(other._max_size),
	_r_status(other._r_status),
	_r_url(other._r_url),
	_err_pages(other._err_pages),
	_locations(other._locations),
	_server_idx(other._server_idx)
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
		_server_idx = other._server_idx;
	}
    return (*this);
}

Server::~Server() {}

Server::Server(std::ifstream &file) :
	_fd(-1),
	_name(""), 
	_ip(""), 
	_port(""), 
	_root(""), 
	_max_size(0),
	_r_status(0),
	_r_url(""),
	_locations()
{
	std::string	line, tok = "", location_path = "";
	
	bool		location_scope = false;
	t_location	location;

	while(getline(file, line))
	{
		std::stringstream	ss(line);
		ss >> tok;

		if (tok == "{" && tok == "\n")
			continue ;
		if (tok == "location" && !location_scope)
		{
			location_scope = true;
			location = t_location();
			location.methods = GET;
			ss >> location_path;
			continue;
		}
		else if(tok == "}")
		{
			if (!location_scope)
				break ;
			this->_locations[location_path] = location;
			location_scope = false;
			tok = "";
		}
		else if (location_scope)
			inputLocation(line, location);
		else
			inputData(line);
	}
	if(this->_root.empty()) throw Error("No root directory found in the server block");
}

int	Server::parse_return(std::stringstream& ss, int& r_status, std::string& r_url)
{
	std::string	val;
		
	if(!(ss >> r_status) || !validateHTTPCode(r_status))
		return (0);
	if (ss >> val)
		r_url = trimSemiColon(val);
	return (1);
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
	std::cout << "err path -> " << err_path << std::endl;
	for (int i = 0; i < temp.size() - 1; ++i)
	{
		int	key = std::atoi(temp[i].c_str());
		if (!validateHTTPCode(key) && key >= 400 && key <= 599)
			return (0);
		err_pg_container[key] = err_path;
		std::cout << "key >> " << key << std::endl;
	}
	return (1);
}

//need to add validation check for the ';'
std::string	Server::trimSemiColon(std::string val)
{
	if(val.find(";") == std::string::npos)
		throw Error("Config failed at " +val);
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
	else if(token == "index"){
		if(line.find(";") != std::string::npos){
			while(ss >> value){
				if(value.find(";") != std::string::npos) _server_idx.push_back(trimSemiColon(value));
				else _server_idx.push_back(value);
			}
		}
		else throw Error("Config failed at" +line);
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
		if(line.find(";") != std::string::npos){
			while(ss >> val)
			{
				if (val.find(";") != std::string::npos) val = trimSemiColon(val);
				location.methods |= identify_method(val);
			}
		}
		else {
			throw Error("Server: Config failed at " + line);}
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
		if(line.find(";") != std::string::npos){
			while(ss >> val){
				if(val.find(";") != std::string::npos) location.index_files.push_back(trimSemiColon(val));
				else location.index_files.push_back(val);
			}
		}
		else throw Error("Server: Config failed at " + line);
		
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
	else if(token == "error_page" && !parse_err_pages(ss, location.err_pages))
		return (0);
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
	_time = time(NULL);
	return (0);
}

bool	Server::is_timeout() const
{
	return (false);
}

void	Server::timeout()
{
	Epoll::instance().del_fd(_fd);
	delete this;
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

long long	Server::max_size() const
{
	return (_max_size);
}

const std::map<std::string, t_location>	&Server::locations() const
{
	return (_locations);
}

const std::map<int, std::string>  &Server::err_pages() const
{
	return (_err_pages);
}

std::vector<std::string> Server::server_idx() const
{
	return (_server_idx);
}

//accepting clients
void	Server::handle(uint32_t events)
{
		Connection	*con = new Connection(this);
		fd	con_fd = *con;
		if (con_fd < 0)
		{
			delete con;
			fail("Server: Client", errno);
		}
		if (Epoll::instance().add_fd(con, con_fd, EPOLLIN) < 0)
		{
			fail("Epoll: Client", errno);
			delete con;
		}
}