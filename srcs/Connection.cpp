#include "Connection.hpp"

Connection::Connection() :
	Pollable(-1),
	_ip(""),
	_port(0),
	_time(0),
	_req(),
	_rep(),
	_server(NULL)
{}

Connection::Connection(const Connection &other) :
	Pollable(other._fd),
	_ip(other._ip),
	_port(other._port),
	_time(other._time),
	_req(other._req),
	_rep(other._rep),
	_server(other._server)
{}

Connection	&Connection::operator=(const Connection &other)
{
	if (this != &other)
	{
		_fd = other._fd;
		_ip = other._ip;
		_port = other._port;
		_time = other._time;
		_req = other._req;
		_rep = other._rep;
		_server = other._server;
	}
	return (*this);
}

Connection::~Connection() {}

Connection::Connection(const Server *server) : _server(server)
{
	
}




