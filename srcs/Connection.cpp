#include "Connection.hpp"
#include "Server.hpp"
#include "Cgi.hpp"

Connection::Connection() :
	_fd(-1),
	_ip(""),
	_port(0),
	_time(0),
	_state(CREATED),
	_req(),
	_rep(),
	_server(NULL),
	_loc(""),
	_cgi(NULL)
{
	_reader = t_reader();
}

Connection::Connection(const Connection &other) :
	_fd(-1),
	_ip(other._ip),
	_port(other._port),
	_time(other._time),
	_state(other._state),
	_req(other._req),
	_rep(other._rep),
	_server(other._server),
	_loc(other._loc),
	_cgi(other._cgi)
{
	_reader = other._reader;
}

Connection	&Connection::operator=(const Connection &other)
{
	if (this != &other)
	{
		_fd = other._fd;
		_ip = other._ip;
		_port = other._port;
		_time = other._time;
		_state = other._state;
		_req = other._req;
		_rep = other._rep;
		_server = other._server;
		_reader = other._reader;
		_loc = other._loc;
		_cgi = other._cgi;
	}
	return (*this);
}

Connection::~Connection()
{
	if (_fd >= 0)
		close(_fd);
	if (_cgi)
	{
		delete _cgi;
		_cgi = NULL;
	}
}

Connection::Connection(const Server *server) :
	_fd(-1),
	_server(server),
	_state(CREATED),
	_reader(t_reader()),
	_ip(""),
	_port(0),
	_time(0),
	_loc(""),
	_cgi(NULL)
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

/* ====================== return the whole location block from the config ======================*/
const t_location*	Connection::find_location(std::string &req_url, std::string &final_path, std::string &remain)
{
	// std::string			loc = "";
	const t_location*	location = NULL;

	for (int i = req_url.size(); i >= 0; --i)
	{
		if (req_url[i] == '/' || i == req_url.size())
		{
			_loc = req_url.substr(0, i);
			location = get(_server->locations(), !i ? "/" : _loc);
			
			if (location)
			{
				std::string	root = location->root.empty() ? _server->root() : location->root;
				remain = req_url.substr(i);
				final_path = root + (_loc == "/" ? "" : _loc) + remain;
				return (location);
			}
		}
	}
	return (NULL);
}

void	Connection::handle(uint32_t events)
{
	if (events & (EPOLLHUP | EPOLLERR))
	{
		fail ("Epoll: ", errno);
		cleanup();
		return;
	}
	if (events & EPOLLIN)
	{
		if (request())
		{
			if (_state == PROCESSING)
			{
				if (Epoll::instance().mod_fd(this, _fd, EPOLLOUT) < 0)
				{
					fail("Epoll: Mod", errno);
					cleanup();
					return ;
				}
				// std::cerr << std::string(40, '=') << "\n" << _reader.header << _reader.body << std::string(40, '=') << std::endl;
				_req.parseRequest((_reader.header + _reader.body).c_str());
				_reader.body = "";
				_reader.header = "";
				_reader.buffer = "";
				_reader.content_len = 0;
				_reader.is_chunked = 0;
				std::cout << "[connection]\tclient request\t\t\t| " << _ip << ":" << _port << " | socket:" << _fd<< " | " 
						  << "method: " << _req.method() << " " << _req.path()<< std::endl;
			}
		}
		else
		{
			cleanup();
			return ;
		}
	}
	if (events & EPOLLOUT)
	{
		if (_state == PROCESSING)
		{
			route();
			std::cout << "conneciton processing done" << std::endl;
			_state = READING_RESPONSE;
		}
		if (_state == READING_RESPONSE)
		{
			if (_cgi)
			{
				if (_cgi->done())
				{
					std::cout << std::string(40, '=') << "\n" << _cgi->output().size() << std::endl;
					_rep._status = 200;
					_rep._type = "text/html";
					_rep._body = _cgi->output();

					if (_cgi)
					{
						delete _cgi;
						_cgi = NULL;
					}
					std::cout << "cgi done" << std::endl;
					_state = DONE;
				}
				return ;
			}
			std::cout << "connection read response" << std::endl;
			_state = DONE;
		}
		if (_state == DONE)
		{
			response();
			cleanup();
		}
	}
}
bool	Connection::read_header()
{
	char	buffer[4096];
	while (true)
	{
		ssize_t bytes = read(_fd, buffer, sizeof(buffer));
		if (bytes > 0)
		{
			_reader.buffer.append(buffer, bytes);
			_time = time(NULL);

			size_t	pos;
			if ((pos = _reader.buffer.find(CRLF + CRLF)) != std::string::npos)
			{
				pos += 4;
				_reader.header = _reader.buffer.substr(0, pos);
				_reader.body = _reader.buffer.substr(pos);
				_reader.is_chunked = _reader.header.find("Transfer-Encoding: chunked\r\n") != std::string::npos;
				if ((pos = _reader.header.find("Content-Length: ")) != std::string::npos)
				{
					std::string	len_str = _reader.header.substr(pos);
					try
					{
						std::stringstream ss(len_str);
						std::string	temp;
						ss >> temp >> _reader.content_len;
					}
					catch(...)
					{
						_reader.content_len = 0;
					}
				}
				if (!_reader.is_chunked && _reader.content_len == 0)
					_state = PROCESSING;
				else if (_reader.content_len > 0 && _reader.body.size() >= _reader.content_len)
					_state = PROCESSING;
				else
					_state = READING_BODY;
				break ;
			}
		}
		else if (bytes == 0)
			return (false);
		else
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return (true);
			fail("Request: Read: Header", errno);
			return (false);
		}
	}
	// std::cout << std::string(40, '=') << "\n" << _reader.header << std::string(40, '=') << std::endl;
	return (true);
}

bool	Connection::request()
{
	char		buffer[4096];

	switch (_state)
	{
		case CREATED: _state = READING_HEADERS;
			
		case READING_HEADERS:
			return (read_header());
		case READING_BODY:
		{
			ssize_t bytes = read(_fd, buffer, sizeof(buffer));
			if (bytes > 0)
			{
				_reader.body.append(buffer, bytes);
				_time = time(NULL);
				if ((_reader.is_chunked && _reader.body.find(CRLF + "0" + CRLF) != std::string::npos) ||
				_reader.content_len > 0 && _reader.body.size() >= _reader.content_len)
					_state = PROCESSING;
				return (true);
			}
			else if (bytes == 0)
				return (false);
			else
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					break ;
				fail("Request: Read: Body", errno);
				return (false);
			}
			if ((_reader.is_chunked && _reader.body.find(CRLF + "0" + CRLF) != std::string::npos) ||
				_reader.content_len > 0 && _reader.body.size() >= _reader.content_len)
				_state = PROCESSING;
			return (true);
		}
		default:
			break;
	}
	return (true);
}

bool	Connection::response()
{
	// _rep._body = status_page(200);
	// _rep._status = 200;
	// _rep._type = "text/html";
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
	std::cout << "[connection]\tclient received response \t| " << _ip << ":" << _port << " | socket:" << _fd << " | "
			  << "method: " << _rep._status << std::endl;
	return (true);
}

/* =================== HANDLE WHICH ROUTE TO HANDLE ================ */
void	Connection::route()
{
	std::string	url = _req.path(), final_path = "", remain_path = "";
	if (_server->r_status())
	{
		redirect_handle(_server->r_status(), _server->r_url(), _rep);
		return ;
	}
	
	const t_location*	location = find_location(url, final_path, remain_path);
	
	if (location)
	{
		const std::string *exec_path = NULL;
		if (!(location->methods & identify_method(_req.method())))
		{
			_rep._status = 405;
			_rep._body = status_page(405);	
			std::cout << "method not allowed " << std::endl;
			return;
		}
		 
		if (location->r_status > 0)
			_req.set_category(REDIRECTION);
		std::cout << "category : " << _req.category() << std::endl;
		if (_req.category() == CGI)
		{
			exec_path = get(location->cgi, "." + get_ext(final_path));
			if (!exec_path)
				_req.set_category(NORMAL);
		}
		if(_req.method() == "DELETE" && _req.category() != CGI)
			_req.set_category(FILEHANDLE);
		switch (_req.category())
		{
			case NORMAL:
				_rep._status = norm_handle(final_path, _req, _rep, location, _loc, _server);
				break;
			case CGI:
				_cgi = new Cgi();
				_cgi->execute(final_path, exec_path, _req);
				break;
			case REDIRECTION:
				redirect_handle(location->r_status, location->r_url, _rep);
				return ;
			case FILEHANDLE:
				_rep._status = handleFile(location, remain_path, _req, _rep);
				std::cout << "filehandle: " << _rep._status << std::endl;
				break;
		}

	}
	if(_server->locations().empty() && _req.path() == "/")
	{
		
		_rep._type = "text/html";
		_rep._status = handleServerIndex(_rep, _server);
		if(_rep._status != 200) _rep._body = status_page(404);
	}
	std::cout << "Connectin rep check >> " << _rep._status << std::endl;
	if (_rep._status >= 400)
	{
		const std::string* err_page;
		std::string err_path;

		_rep._type = "text/html";

		if (!_server->err_pages().empty())
		{
			err_page = get(_server->err_pages(), _rep._status);
			if (err_page)
				err_path = _server->root() + *err_page;
			std::cout << "error path 1" << err_path << std::endl;
		}
		else if (!location->err_pages.empty())
		{
			err_page = get(location->err_pages, _rep._status);
			if (err_page){
				if(!location->root.empty())
					err_path = location->root + *err_page;
				else{
					if(!_server->root().empty())
						err_path = _server->root() + *err_page; //don't need "else" bcuz config will not work if no root in parsing
				}
			}
			std::cout << "error path 2" << err_path << std::endl;
		}
		if (!err_path.empty())
		{
			int status = read_file(err_path, _rep._body);
			if (status != 200)
				_rep._status = status;
			else
				return ;
		}
		else	_rep._status = 403;
		_rep._body = status_page(_rep._status);
	}

}

void	Connection::cleanup()
{
	Epoll::instance().del_fd(_fd);
	std::cout << "[connection]\tclient disconnected\t\t| socket:" << _fd << std::endl;
	if (_fd >= 0)
	{
		close(_fd);
		_fd = -1;
	}
	delete	this;
}

std::time_t	Connection::con_time() const
{
	std::time_t	now = time(0);
	return (now - _time);
}

Connection::operator std::time_t() const
{
	return (_time);
}

Connection::operator	fd() const
{
	return (_fd);
}

void	Connection::set_req(Request &req)
{
	_req = req;
}

void	Connection::set_server(Server *server)
{
	_server = server;
}


