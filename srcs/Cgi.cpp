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

Cgi::~Cgi() {}

int	Cgi::execute(std::string& final_path, const std::string* exec_path, Request &req)
{
	int	in_pipe[2], out_pipe[2];

	std::vector<std::string> envs = req.cgi_env();
	std::vector<char*>	envp;

	for (size_t i = 0; i < envs.size(); ++i)
		envp.push_back(const_cast<char*>(envs[i].c_str()));
	envp.push_back(NULL);

	// if (pipe(in_pipe) < 0)
	// 	return (fail("CGI: File", errno), 500);
	if (pipe(out_pipe) < 0)
	{
		fail("CGI: Pipe", errno);	
		// close(in_pipe[0]);
		// close(in_pipe[1]);
		return (500);
	}
	_pid = fork();
	if (_pid == -1)
		return (fail("CGI: Fork", errno));
	if (_pid == 0)
	{
		// dup2(in_pipe[0], STDIN_FILENO);
		dup2(out_pipe[1], STDOUT_FILENO);

		// close(in_pipe[1]);
		close(out_pipe[0]);
		// close(in_pipe[0]);
		close(out_pipe[1]);

		char	*argv[] = {const_cast<char*>(exec_path->c_str()),
						   const_cast<char*>(final_path.c_str()), NULL};
		if (execve(exec_path->c_str(), argv, const_cast<char* const*>(&envp[0])) == -1)
		{
			exit(1);
			perror("execl failed");
		}
 	}
	else
	{
		// close(in_pipe[0]);
		close(out_pipe[1]);

		// _in_fd = in_pipe[1];
		_out_fd = out_pipe[0];
		_body = req.body();

		// set_nblocking(_in_fd);
		set_nblocking(_out_fd);
		if (Epoll::instance().add_fd(this, _out_fd, EPOLLIN) < 0)
		{
			fail("CGI: Epoll: output", errno);
			close(_out_fd);
			close(_in_fd);
			_in_fd = _out_fd = -1;
			return (500);
		}
		if ((identify_method(req.method()) & POST) && !_body.empty())
		{
			if (Epoll::instance().add_fd(this, _in_fd, EPOLLOUT) < 0)
			{
				fail("CGI: Epoll: output", errno);
				close(_out_fd);
				close(_in_fd);
				_in_fd = _out_fd = -1;
				return (500);
			}
			_state = CGI_WRITING;
		}
		else
		{
			// close(_in_fd);
			// _in_fd = -1;
			// _state = CGI_READING;
		}
	}
	return (200);
}

void	Cgi::handle(uint32_t events)
{
	// std::cout << "Cgi handle" << std::endl;
	// if ((events & EPOLLOUT) && _state == CGI_WRITING)
	// {
	// 	std::cout << "cgi input : body_size : " << _body.size() << " state : " << _state << std::endl;
	// 	size_t	remian = _body.size() - _written;
	// 	if (remian == 0)
	// 	{
	// 		std::cout << "cgi input is done" << std::endl;
	// 		Epoll::instance().del_fd(_in_fd);
	// 		close(_in_fd);
	// 		_in_fd = -1;
	// 		_state  = CGI_READING;
	// 		return ;
	// 	}
	// 	size_t	n = ::write(_in_fd, _body.data() + _written, remian);
	// 	if (n > 0)
	// 	{
	// 		_written += static_cast<size_t>(n);
	// 		std::cout << "cgi input, n : " << n << " written: " << _written << std::endl;
	// 		// if (_written >= _body.size())
	// 		// {
	// 		// 	std::cout << "cinput 1 " << std::endl;
	// 			Epoll::instance().del_fd(_in_fd);
	// 			close(_in_fd);
	// 			_in_fd = -1;
	// 			_state = CGI_READING;
	// 		// }
	// 	}
	// 	if (n < 0)
	// 	{
	// 		if (errno == EAGAIN || errno == EWOULDBLOCK)
	// 			return ;
	// 		fail("Cgi: Input", errno);
	// 		Epoll::instance().del_fd(_in_fd);
	// 		close(_in_fd);
	// 		_in_fd = -1;
	// 		_state = CGI_ERROR;
	// 	}
	// 	// if (_written >= _body.size())
	// 	// {
	// 	// 	std::cout << "cinput 3 " << std::endl;
	// 	// 	std::cout << "cgi writing"  << std::endl;
	// 	// 	Epoll::instance().del_fd(_in_fd);
	// 	// 	close(_in_fd);
	// 	// 	_in_fd = -1;
	// 	// 	_state = CGI_READING;
	// 	// 	std::cout << "cgi input done" << std::endl;
	// 	// 	return ;
	// 	// }
	// 	// std::cout << "cinput 4 " << remian << std::endl;
	// 	// _state = CGI_READING;
	// }
	if ((events & EPOLLIN) && _out_fd != -1)
	{
		std::cout << "cgi epollin : state " << _state << std::endl;

		char 	buffer[2];
		size_t	total = 0;
		while (total < 1)
		{
			ssize_t n = ::read(_out_fd, buffer, sizeof(buffer));
			// std::cout << "n: " << n << " errno: " << errno << std::endl;
			if (n > 0)
			{
				_output.append(buffer, static_cast<size_t>(n));
				total += n;
				std::cout << "cgi output, total: " << total << std::endl;

			}
			else if (n == 0)
			{
				std::cout << "omg it's here " << std::endl;
				// std::cout << std::string(40, '=') << "\n" << _output << std::string(40, '=') << "\n" << std::endl;
				Epoll::instance().del_fd(_out_fd);
				close(_out_fd);
				_out_fd = -1;

				// Optional: reap child without blocking.
				// If child not exited yet, this returns 0 and we ignore.
			}
			else if (n < 0 && !(errno == EAGAIN || errno == EWOULDBLOCK))
			{
				fail("Cgi: output", errno);
				std::cout << "i dk " << std::endl;
				Epoll::instance().del_fd(_out_fd);
				close(_out_fd);
				_out_fd = -1;
				_state = CGI_DONE;
				return ;
			}
			std::cout << "looping" << std::endl;
		}
	}
	if (_pid > 0)
	{
		int status;
		pid_t ret = waitpid(_pid, &status, WNOHANG);
		if (ret == _pid)
		{
			// Child exited
			_state = CGI_DONE;
			_pid = -1;
			if (_out_fd != -1)
			{
				Epoll::instance().del_fd(_out_fd);
				close(_out_fd);
				_out_fd = -1;
			}
		}
		else if (ret == -1)
		{
			perror("waitpid");
			_state = CGI_DONE;
		}
		// ret == 0 → child still running, do nothing
	}
}

bool	Cgi::done()
{
	return (_state == CGI_DONE);
}

std::string	Cgi::output()
{
	return (_output);
}