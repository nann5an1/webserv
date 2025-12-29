#include "CGI.hpp"
#include "Epoll.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <cstring>

static void set_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0)
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

CGI::CGI()
    : _pid(-1),
      _in_fd(-1),
      _out_fd(-1),
      _state(CGI_CREATED),
      _written(0)
{
}

CGI::~CGI()
{
    if (_in_fd >= 0)  close(_in_fd);
    if (_out_fd >= 0) close(_out_fd);
}

int	CGI::execute(std::string &final_path, const t_location *location, Request& req, Response& rep)
{
    fd	in_pipe[2];
    fd	out_pipe[2];
	int	status;

	std::vector<std::string>	cgi_env = req.cgi_env();
	std::vector<const char *>	env;
	for (int i = 0; i < cgi_env.size(); ++i)
		env.push_back(cgi_env[i].c_str());
	env.push_back(NULL);

	const std::string	*exec_path = get(location->cgi, "." + get_ext(final_path));
	if (!exec_path)
		return (404);
	if ((status = file_check(final_path, X_OK)) != 200)
		return (fail("CGI: File", errno), status);
    if (pipe(in_pipe) == -1)
		return (fail("CGI: Pipe", errno), 500);
	if (pipe(out_pipe) == -1)
	{
		fail("CGI: Pipe", errno);
		if (in_pipe[0])
			close(in_pipe[0]);
		if (in_pipe[1])
			close(in_pipe[1]);
		return (500);
	}
    pid_t pid = fork();
    if (pid < 0)
        return (fail("CGI: Fork", errno), 500);
    if (pid == 0)
    {
        dup2(in_pipe[0], STDIN_FILENO);
        dup2(out_pipe[1], STDOUT_FILENO);

        close(in_pipe[0]);
        close(in_pipe[1]);
        close(out_pipe[0]);
        close(out_pipe[1]);

        char *argv[] = {
            const_cast<char*>(exec_path->c_str()),
            const_cast<char*>(final_path.c_str()),
            NULL
        };

        execve(exec_path->c_str(), argv, const_cast<char* const*>(&env[0]));
        _exit(1);
    }

    // parent
    close(in_pipe[0]);
    close(out_pipe[1]);

    set_nonblock(in_pipe[1]);
    set_nonblock(out_pipe[0]);

    _pid     = pid;
    _in_fd   = in_pipe[1];
    _out_fd  = out_pipe[0];
    _written = 0;
    _state   = CGI_WRITING;

    return (true);
}

void CGI::handle(uint32_t events)
{
    if (_state == CGI_WRITING && (events & EPOLLOUT))
    {
        ssize_t n = write(_in_fd,
                          _body.data() + _written,
                          _body.size() - _written);

        if (n > 0)
        {
            _written += n;
            if (_written == _body.size())
            {
                close(_in_fd);
                _in_fd = -1;
                _state = CGI_READING;
            }
        }
        else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
        {
            _state = CGI_ERROR;
        }
    }

    if (_state == CGI_READING && (events & EPOLLIN))
    {
        char buf[4096];
        ssize_t n = read(_out_fd, buf, sizeof(buf));

        if (n > 0)
            _output.append(buf, n);
        else if (n == 0)
        {
            close(_out_fd);
            _out_fd = -1;

            int status;
            waitpid(_pid, &status, WNOHANG);
            _state = CGI_DONE;
        }
        else if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            _state = CGI_ERROR;
        }
    }
}

bool CGI::done() const
{
    return _state == CGI_DONE;
}

bool CGI::error() const
{
    return _state == CGI_ERROR;
}

int CGI::in_fd() const
{
    return _in_fd;
}

int CGI::out_fd() const
{
    return _out_fd;
}

const std::string &CGI::output() const
{
    return _output;
}
