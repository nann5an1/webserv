#include "Connection.hpp"

Connection::Connection() : _fd(-1) {}

Connection::Connection(const Connection &other) : _fd(other._fd) {}

Connection	&Connection::operator=(const Connection &other)
{
	if (this != &other)
	{
		_fd = other._fd;
		_time = other._time;
	}
	return (*this);
}

Connection::~Connection()
{
}

Connection::Connection(fd server_fd)
{
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
}

Connection::operator	fd() const
{
	return (_fd);
}

Connection::operator std::time_t() const
{
	return (_time);
}

bool	Connection::getRequest()
{
	char	buffer[4096];
	while (true)
	{
		ssize_t bytes = read(_fd, buffer, sizeof(buffer));
		if (bytes > 0)
		{
			std::string	req(buffer, bytes);
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
	return (true);
}

bool	Connection::response()
{
	const char	*str = _rep;
	if (write(_fd, str, std::strlen(str)) < 0 && errno != EAGAIN)
		return (fail("Response", errno), 1);
	return (0);
}
