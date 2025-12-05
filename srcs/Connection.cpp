#include "Connection.hpp"

Connection::Connection() : _fd(-1), _time(0), _rep() {}

Connection::Connection(const Connection &other) : _fd(other._fd), _time(other._time), _rep(other._rep) {}

Connection	&Connection::operator=(const Connection &other)
{
	if (this != &other)
	{
		_fd = other._fd;
		_time = other._time;
		_rep = other._rep;
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
	std::cout << "[connection]\tclient connected\t\t| socket:" << _fd << std::endl;

}

Connection::operator	fd() const
{
	return (_fd);
}

Connection::operator std::time_t() const
{
	return (_time);
}

bool	Connection::request()
{
	char	buffer[4096];
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
	std::cout << "[connection]\tclient request\t\t\t| socket:" << _fd << "\n\n"
			  << req << std::endl;
	return (true);
}

bool	Connection::response()
{
	const char* str = _rep.build();
    size_t size = std::strlen(str);
    ssize_t n = write(_fd, str, size);

    if (n < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return false;   // wait for next epoll notification
        return fail("Response", errno), true;
    }

    // all bytes sent (or small responses handled in one write)
    std::cout << "[connection]\tclient received response | socket:" << _fd << "\n\n" << str << std::endl;
    return true;
}
