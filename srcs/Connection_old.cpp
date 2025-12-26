#include "Connection.hpp"

Connection::Connection() : _fd(-1), _time(0), _rep(), _server(NULL), _ip(""), _port(0) {}

Connection::Connection(const Connection &other) : _fd(other._fd), _time(other._time), _rep(other._rep), _server(other._server), _ip(other._ip), _port(other._port) {}

Connection	&Connection::operator=(const Connection &other)
{
	if (this != &other)
	{
		_fd = other._fd;
		_time = other._time;
		_rep = other._rep;
		_server = other._server;
		_ip = other._ip;
		_port = other._port;
	}
	return (*this);
}

Connection::~Connection() {}

Connection::Connection(const Server *server) : _server(server)
{
	fd	server_fd = *_server;
	sockaddr_in	client_addr;
	socklen_t	client_len = sizeof(client_addr);
	_fd = accept(server_fd, (sockaddr *)&client_addr, &client_len);
	if (_fd < 0 )
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			fail("Epoll", errno);
		return ;
	}
	_ip = inet_ntoa(client_addr.sin_addr);
	_port = ntohs(client_addr.sin_port);

	// FAIL:
	if (fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL, 0) | O_NONBLOCK) < 0)
	{
		fail("Connection", errno);
		if (_fd > 0)
			close(_fd);
		_fd = -1;
		return;
	}
	_time = time(NULL);
	std::cout << "[connection]\tclient connected\t\t| " << _ip << ":" << _port << " | socket:" << _fd << std::endl;

}

bool	Connection::request()
{
	char		buffer[4096];
	std::string	req;

	while (true)
	{
		ssize_t bytes = read(_fd, buffer, sizeof(buffer));
		if (bytes > 0)
		{
			req += std::string(buffer, bytes);
			_time = time(NULL);
		}
		else if (bytes == 0)
			return (false);
		else
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			return (false);
		}
	}
	_req.parseRequest(req.c_str());
	std::cout << "[connection]\tclient request\t\t\t| socket:" << _fd << "\n\n"
			  << req << std::endl;
	std::cout << "===================================================\n\n\n\n" << std::endl;
	_req.parseRequest(req.c_str());	
	return (true);
}

bool	Connection::response()
{
	route();
	const char* str = _rep.build();
	size_t size = std::strlen(str);
	ssize_t n = write(_fd, str, size);

	if (n < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (false);   // wait for next epoll notification
		return (fail("Response", errno), true);
	}

	// all bytes sent (or small responses handled in one write)
	std::cout << "[connection]\tclient received response \t| socket:" << _fd << "\n\n" << str << std::endl;
	return (true);
}

/* ====================== return the whole location block from the config ======================*/
const t_location*	Connection::find_location(std::string &req_url, std::string &final_path)
{
	std::string	loc = "";
	const t_location*	location = NULL;

	for (int i = req_url.size(); i >= 0; --i)
	{
		if (req_url[i] == '/' || i == req_url.size())
		{
			loc = req_url.substr(0, i);
			location = get(_server->locations(), !i ? "/" : loc);
			if (location)
			{
				std::string	root = location->root.empty() ? _server->root() : location->root;
				final_path = root + (loc == "/" ? "" : loc) + req_url.substr(i);
				return (location);
			}
		}
	}
	
	return (NULL);
}



void	Connection::route()
{
	std::string	url = _req.path(), final_path = "";

	if (_server->r_status())
	{
		redirect_handle(_server->r_status(), _server->r_url(), _rep);
		return ;
	}

	const t_location*	location = find_location(url, final_path);
	std::cout << "final: " << final_path << ", category: " << _req.category() << std::endl;
 
	if (location)
	{
		if (!(location->methods & identify_method(_req.method())))
		{
			std::cout << "method not allowed " << std::endl;
		}
		if (location->r_status > 0)
			_req.set_category(REDIRECTION);
		switch (_req.category())
		{
			case NORMAL:
				_rep._status = norm_handle(final_path, _req, _rep, location);
				break;
			case CGI:
				std::cout << "cgi request come in " << std::endl;
				cgi_handle(final_path, location, _req, _rep);
				_rep._status = 200;
				break;
			case REDIRECTION:
				redirect_handle(location->r_status, location->r_url, _rep);
				return ;
			case FILEHANDLE:
				_rep._status = 200;
				handleFile(location, _req, _rep);
				std::cout << "File upload come in" << std::endl;
				break;
		}
	}
	else
	{
		_rep._type = "text/html";
		_rep._status = 404;
		_rep._body = status_page(404);
	}

	// final = root + path;
	// error shold handle here;

	
	// if (_rep._status >= 400)
	// 	error_handle();
	// std::cout << "this is the status: " << _rep._status << std::endl;
}

std::time_t	Connection::contime() const
{
	std::time_t	now = time(0);
	return (now - _time);
}

Connection::operator	fd() const
{
	return (_fd);
}

Connection::operator std::time_t() const
{
	return (_time);
}

void	Connection::set_req(Request &req)
{
	_req = req;
}

void	Connection::set_server(Server *server)
{
	_server = server;
}
