#include "Cgi.hpp"
#include "Epoll.hpp"

Cgi::Cgi() : _pid(-1), _in_fd(-1), _out_fd(-1), _body(""), _written(0), _state(CGI_WRITING) {}

Cgi::Cgi(const Cgi &other) : _pid(-1), _in_fd(other._in_fd), _out_fd(other._out_fd), _body(other._body), _written(other._written), _state(other._state) {}

Cgi	&Cgi::operator=(const Cgi &other)
{
	if (this != &other)
	{
		_pid = other._pid;
		_in_fd = other._in_fd;
		_out_fd = other._out_fd;
		_body = other._body;
		_written = other._written;
		_state = other._state;
	}
	return (*this);
}

Cgi::~Cgi() 
{
	 if (_pid > 0)
    {
        ::kill(_pid, SIGKILL);
        ::waitpid(_pid, NULL, 0);
    }
    if (_in_fd >= 0)
        close_fd(_in_fd);
    if (_out_fd >= 0)
        close_fd(_out_fd);
}

void	Cgi::close_fd(int &fd_)
{
	if (fd_ != -1)
	{
		Epoll::instance().del_fd(fd_);
		close(fd_);
		fd_ = -1;
	}
}


int	Cgi::execute(std::string& final_path, const std::string* exec_path, Request &req)
{
	int	in_pipe[2], out_pipe[2];

	_time = time(NULL);
	_body = req.body();
	_written = 0;
	_output.clear();
	_state = CGI_WRITING;

	std::vector<std::string> envs = req.cgi_env();
	std::vector<char*>	envp;

	for (size_t i = 0; i < envs.size(); ++i)
		envp.push_back(const_cast<char*>(envs[i].c_str()));
	envp.push_back(NULL);

	if (::pipe(in_pipe) < 0)
		return (fail("CGI: File", errno), 500);
	if (::pipe(out_pipe) < 0)
	{
		fail("CGI: Pipe", errno);	
		close(in_pipe[0]);
		close(in_pipe[1]);
		return (500);
	}
	_pid = fork();
	if (_pid == -1)
	{
		close(in_pipe[0]);
		close(in_pipe[1]);
		close(out_pipe[0]);
		close(out_pipe[1]);
		return (fail("CGI: Fork", errno), 500);
	}
	if (_pid == 0)
	{
		if (dup2(in_pipe[0], STDIN_FILENO) == -1 || dup2(out_pipe[1], STDOUT_FILENO) == -1)
			_exit(1);
		close(in_pipe[1]);
		close(out_pipe[0]);
		close(in_pipe[0]);
		close(out_pipe[1]);

		char	*argv[] = {const_cast<char*>(exec_path->c_str()),
						   const_cast<char*>(final_path.c_str()), 
						   NULL};
		execve(exec_path->c_str(), argv, const_cast<char* const*>(&envp[0]));
		_exit(1);
 	}
	else
	{
		close(in_pipe[0]);
		close(out_pipe[1]);

		_in_fd = in_pipe[1];
		_out_fd = out_pipe[0];

		set_nblocking(_out_fd);
		if (Epoll::instance().add_fd(this, _out_fd, EPOLLIN | EPOLLHUP | EPOLLERR) < 0)
		{
			fail("CGI: Epoll: output", errno);
			close(_out_fd);
			close(_in_fd);
			_in_fd = _out_fd = -1;
			return (500);
		}
		if ((identify_method(req.method()) & POST) && !_body.empty())
		{
			set_nblocking(_in_fd);
			if (Epoll::instance().add_fd(this, _in_fd, EPOLLOUT | EPOLLHUP | EPOLLERR) < 0)
			{
				fail("CGI: Epoll: output", errno);
				close_fd(_out_fd);
				close(_in_fd);
				_in_fd = -1;
				return (500);
			}
			_state = CGI_WRITING;
		}
		else
		{
			close(_in_fd);
			_in_fd = -1;
			_state = CGI_READING;
		}
	}
	return (200);
}

void	Cgi::handle(uint32_t events)
{
	if (events & EPOLLERR)
	{
		close_fd(_out_fd);
		close_fd(_in_fd);
		_state = CGI_ERROR;
		if (_pid > 0)
		{
			int status = 0;
			pid_t ret = waitpid(_pid, &status, WNOHANG);
			if (ret == _pid)
				_pid = -1;
		}
		return ;
	}
	if ((events & EPOLLOUT) && _state == CGI_WRITING)
	{
		std::cout << "cgi input : body_size : " << _body.size() << " state : " << _state << std::endl;
		size_t	remain = _body.size() - _written;
		if (remain == 0)
		{
			close_fd(_in_fd);
			_state  = CGI_READING;
			return ;
		}
		ssize_t	n = ::write(_in_fd, _body.data() + _written, remain);
		if (n > 0)
		{
			_written += static_cast<size_t>(n);
			if (_written >= _body.size())
			{
				close_fd(_in_fd);
				_state = CGI_READING;
			}
		}
		else if (n == 0)
			return ;
		else if (n < 0 && !(errno == EAGAIN || errno == EWOULDBLOCK))
		{
			fail("Cgi: Input", errno);
			close_fd(_in_fd);
			_state = CGI_ERROR;
		}
	}
	if ((events & (EPOLLIN | EPOLLHUP)) && _out_fd != -1)
	{
		char 	buffer[4096];
		while (true)
        {
            ssize_t n = ::read(_out_fd, buffer, sizeof(buffer));
            if (n > 0)
            {
                _output.append(buffer, static_cast<size_t>(n));
                continue;
            }
            if (n == 0)
            {
                close_fd(_out_fd);
				_state = CGI_DONE;
                break;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            fail("Cgi: Output", errno);
            close_fd(_out_fd);
            _state = CGI_ERROR;
            break;
        }
	}
	if (_pid > 0)
	{
		int status = 0;
		pid_t ret = waitpid(_pid, &status, WNOHANG);
		if (ret == _pid)
			_pid = -1;
	}
	if (_state != CGI_ERROR)
	{
		if (_out_fd == -1 && _in_fd == -1 && _pid == -1)
			_state = CGI_DONE;
	}
	// if (_pid > 0)
	// {
	// 	int status;
	// 	pid_t ret = waitpid(_pid, &status, WNOHANG);
	// 	if (ret == _pid)
	// 	{
	// 		// Child exited
	// 		_state = CGI_DONE;
	// 		_pid = -1;
	// 		if (_out_fd != -1)
	// 		{
	// 			Epoll::instance().del_fd(_out_fd);
	// 			close(_out_fd);
	// 			_out_fd = -1;
	// 		}
	// 		if (_in_fd != -1)
	// 		{
	// 			Epoll::instance().del_fd(_in_fd);
	// 			close(_in_fd);
	// 			_in_fd = -1;
	// 		}
	// 	}
	// 	else if (ret == -1)
	// 	{
	// 		fail("Cgi: waitpid", errno);
	// 		_state = CGI_DONE;
	// 	}
	// 	// ret == 0 → child still running, do nothing
	// }
}

std::string	Cgi::output()
{
	return (_output);
}

std::time_t	Cgi::get_time() const
{
	std::time_t	now = time(0);
	return (now - _time);
}

bool	Cgi::is_timeout() const
{
	return (get_time() >= CGI_TIMEOUT);
}

void	Cgi::cleanup()
{
	if (_pid > 0)
	{
		::kill(_pid, SIGKILL);
		::waitpid(_pid, NULL, 0);
		_pid = -1;
	}
	close_fd(_in_fd);
	close_fd(_out_fd);
	_state = CGI_KILL;
}

void	Cgi::timeout()
{
	cleanup();
}

cgi_state	Cgi::state() const
{
	return (_state);
}