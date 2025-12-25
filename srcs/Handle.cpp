#include "Handle.hpp"

std::string	status_page(int status)
{
	std::string	status_str = to_string(status), phrase = gphrase[status];
	std::string	content = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n\t<meta charset=\"UTF-8\">\n\t<title>Status Page</title>\n</head>\n<body>\n\t<h1>" + status_str + " " + phrase + "</h1>\n</body>\n</html>";
	return (content);
}

int	norm_handle(std::string	&path, Request &req, Response &rep, const t_location* location)
{
	int	status;
	const std::vector<std::string>	&indexs = location->index_files;
	
	if (is_dir(path))
	{
		if (indexs.empty() && location->autoindex)
			return (std::cout << "autoindex" << std::endl, 200);
		for (int i = 0; i < indexs.size(); ++i)
		{
			if (file_check(path + "/" + indexs[i], R_OK) == 200)
			{
				path += "/" + indexs[i];
				goto response;
			}
		}
		return (403);
	}
	response:
		status = file_check(path, R_OK);
		if (status == 200)
		{
			status = read_file(path, rep._body);
			rep._type = "text/" + get_ext(path);
			return (status);
		}
		return (status);
	// status = file_check(path, R_OK);
}

void	redirect_handle(int status, const std::string &path, Response& rep)
{
	rep._body = status_page(status);
	rep._type = "text/html";
	rep._status = status;
	
	std::cout << "got in redirect" << std::endl;
	if (status > 300 && status < 400)
		rep._location = path;
	else if (!path.empty())
	{
		std::cout << "got in here " << std::endl;
		rep._body = path;
		rep._type = "text/plain";
	}
}

int	cgi_handle(std::string &final_path, const t_location *location, Request& req, Response& rep)
{
	fd	in_pipe[2];
	fd	out_pipe[2];
	int	status;

	std::vector<const char*> abc = req.cgi_env();
	std::cout << "size:  " << abc.size() << std::endl;
	// for (int i = 0; i < abc.size(); ++i)
	// {
	// 	std::cout << abc[0] << std::endl;
	// }
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
    if (pid == -1)
		return (fail("CGI: Fork", errno));

    if (pid == 0)
    {
        dup2(in_pipe[0], STDIN_FILENO);
        dup2(out_pipe[1], STDOUT_FILENO);

        close(in_pipe[1]);
        close(out_pipe[0]);
	
		char	*argv[] = {const_cast<char *>(exec_path->c_str()), const_cast<char *>(final_path.c_str()), NULL};
		execve(exec_path->c_str(), argv, const_cast<char* const*>(&req.cgi_env()[0]));
        _exit(1);
    }
    else
    {
        close(in_pipe[0]);
        close(out_pipe[1]);
        if (!req.body().empty())
            write(in_pipe[1], req.body().c_str(), req.body().size());
        close(in_pipe[1]); // close writing end to signal EOF

        // Optionally read CGI output
        char buffer[4096];
        ssize_t n;
        while ((n = read(out_pipe[0], buffer, sizeof(buffer))) > 0) {
            std::cout.write(buffer, n);
        }
        close(out_pipe[0]);

        // Wait for child to finish
        int status;
        waitpid(pid, &status, 0);
    }
	return (0);
}

// void	error_handle()
// {

// }