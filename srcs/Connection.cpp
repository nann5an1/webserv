#include "Connection.hpp"

Connection::Connection() : _fd(-1), _time(0), _rep(), _server(NULL) {}

Connection::Connection(const Connection &other) : _fd(other._fd), _time(other._time), _rep(other._rep), _server(other._server) {}

Connection	&Connection::operator=(const Connection &other)
{
	if (this != &other)
	{
		_fd = other._fd;
		_time = other._time;
		_rep = other._rep;
		_server = other._server;
	}
	return (*this);
}

Connection::~Connection()
{
}

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
	std::cout << "[connection]\tclient connected\t\t| socket:" << _fd << std::endl;

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

void	Connection::route()
{
	std::string	path = _req.path();
	
	std::string	final = "", loc = "", root = _server->root();

	if (_server->r_status())
	{
		redirect_handle(_server->r_status(), _server->r_url(), _rep);
		return ;
	}
	std::cout << "path: " << path << std::endl;
	const t_location*	location = NULL;

	location = get(_server->locations(), path.empty() ? "/" : path);

	for (int i = path.size(); i >= 0; --i)
	{

		if (path[i] == '/' || i == path.size())
		{
			loc = path.substr(0, i);
			std::cout << "loc: " << loc << std::endl;
			location = get(_server->locations(), loc.empty() ? "/" : loc);
			if (location)
			{
				root = location->root.empty() ? _server->root() : location->root;
				final = root;
				if (loc != "/")
					final += loc;            // append matched location
				if (i + 1 < path.size())
					final += path.substr(i); // append remaining path after slash
				break;
			}
		}
	}
	if (!location)
	{
		final = root + path;
		// error shold handle here;
	}
	std::cout << "loc: " << loc << ", final: " << final << std::endl;
	if (location->r_status > 0)
		_req.set_category(REDIRECTION);
	switch (_req.category())
	{
		case NORMAL:
			_rep._status = norm_handle(final, _req, _rep, location);
			break;
		case CGI: break;

		case REDIRECTION:
			redirect_handle(location->r_status, location->r_url, _rep);
			return ;
		case UPLOAD: break;

		case AUTOINDEX: break;
	}
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
