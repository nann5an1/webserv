#include "Handle.hpp"

std::string	status_page(int status)
{
	std::string	status_str = to_string(status), phrase = gphrase[status];
	std::string	content = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n\t<meta charset=\"UTF-8\">\n\t<title>Status Page</title>\n</head>\n<body>\n\t<h1>" + status_str + " " + phrase + "</h1>\n</body>\n</html>";
	return (content);
}

/* ================ READ THE ENTIRE DIRECTORY AND LIST DOWN ================*/
std::string autoIndexOnListing(const Server *server, const t_location* location,
	std::string& path)
{	
	size_t remain_len;
	std::string html, ret_str;
    DIR* dir = opendir(path.c_str());
    if (!dir)
        return ("");
	
	if(!location->root.empty())
		remain_len = path.length() - location->root.length();
	else
		remain_len = path.length() - server->root().length();
	ret_str = path.substr(path.length() - remain_len, remain_len);

	html += "<html><head>Index listing of " + path + "</head>";
	html += "<body>";
	html += "<ul>";

	dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		if (std::strcmp(entry->d_name, ".") == 0 || //skil the . and ..
			std::strcmp(entry->d_name, "..") == 0)
			continue;

		html += "<li><a href=\"";
		html +=  ret_str + "/" + entry->d_name;

		if (entry->d_type == DT_DIR)
			html += "/";

		html += "\">";
		html += entry->d_name;

		if (entry->d_type == DT_DIR)
			html += "/";

		html += "</a></li>";
	}

	html += "</ul></body></html>";

	closedir(dir);
	return html;
}

int handleServerIndex(Response &rep, const Server *server)
{
	std::vector<std::string> server_idx = server->server_idx();
	std::string server_path = server->root(), index_path;
	int status;

	// std::cout << "Server indexes size -> " << server_idx.size() << std::endl; 
	// std::cout << "Server path -> " << server_path << std::endl;
	DIR* dir = opendir(server->root().c_str());

	if(!dir)
		return (404);

	for (int i = 0; i < server_idx.size(); ++i)
	{
		index_path = server_path + "/" + server_idx[i];
		if ((status = file_check(index_path, R_OK)) == 200)
		{
			std::cout << "serverindex" << std::endl;
			status = read_file(index_path, rep._body);
			rep._type = mime_types[get_ext(index_path)];
			return (status);
		}
	}
	closedir(dir);
	return (403);
}

int norm_handle(std::string &final_path, Request &req, Response &rep,
                const t_location *location, std::string loc,
                const Server *server)
{
    std::string path = final_path;
    int status;

	std::cout << "path " << path << std::endl;
    // 1. If path is a directory, try index files
    if (is_dir(path))
    {
        // 1.1 Location-level index
        if (!location->index_files.empty())
        {
            for (size_t i = 0; i < location->index_files.size(); ++i)
            {
                std::string index_path = path + "/" + location->index_files[i];
                if (file_check(index_path, R_OK) == 200)
                {
                    status = read_file(index_path, rep._body);
                    rep._type = mime_types[get_ext(index_path)];
                    return status;
                }
            }
        }

        // 1.2 Autoindex
        if (location->autoindex)
        {
            rep._body = autoIndexOnListing(server, location, path);
            if (rep._body.empty())
                return 403;

            rep._type = "text/html";
            return 200;
        }

		if (location->index_files.empty())
		{
			const std::vector<std::string> &srv_idx = server->server_idx();
			for (size_t i = 0; i < srv_idx.size(); ++i)
			{
				std::string index_path = path + "/" + srv_idx[i];
				if (file_check(index_path, R_OK) == 200)
				{
					status = read_file(index_path, rep._body);
					rep._type = mime_types[get_ext(index_path)];
					return status;
				}
			}
		}

        // 1.4 Directory access forbidden
        return 403;
    }


    // 2. Regular file request
    status = file_check(path, R_OK);
    if (status != 200)
        return status;

    status = read_file(path, rep._body);
    rep._type = mime_types[get_ext(path)];
    return status;
}


void	redirect_handle(int status, const std::string &path, Response& rep)
{
	std::cout << "shit got in " << std::endl;
	if (status >= 300)
		rep._body = status_page(status);
	rep._type = "text/html";
	rep._status = status;
	
	if (status > 300 && status < 400)
	{
		if (!path.empty())
			rep._body.clear();
		rep._location = path;
	}
	else if (!path.empty())
	{
		rep._body = path;
		rep._type = "text/plain";
	}
}

int	cgi_handle(std::string &final_path, const t_location *location, Request& req, Response& rep)
{
	fd	in_pipe[2];
	fd	out_pipe[2];
	int	status;

	std::vector<std::string>	cgi_env = req.cgi_env();
	std::vector<const char *>	env;
	for (int i = 0; i < cgi_env.size(); ++i)
		env.push_back(cgi_env[i].c_str());
	env.push_back(NULL);
	// char *const *omg = const_cast<char* const*>(&env[0]);
	// for (int i = 0; i < cgi_env.size(); ++i)
	// 	std::cout << omg[i] << std::endl;

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
		execve(exec_path->c_str(), argv, const_cast<char* const*>(&env[0]));
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

bool fileExists(std::string &path)
{
	std::cout << "getter path >> " << path << std::endl;
	std::ifstream file(path.c_str());
	return file.good();
}

// Add this helper function first
std::string size_to_string(off_t size)
{
	std::ostringstream oss;
	oss << size;
	return oss.str();
}


/* ====================== add the data from the upload_files of the server into the server's upload_dir ======================*/
int	handleFile(const t_location* location, std::string &remain_path, Request &req, Response &rep){
	std::string filepath;
	std::string method = req.method();
	int status;

	// std::cout << "method in handleFile" << method << std::endl;
	if(method == "POST" && req.upload_files().size() > 0)
	{ //METHOD = POST
		std::vector<binary_file> files = req.upload_files();
		std::cout << "files size DEBUG >> " << files.size() << std::endl;
		
		//iterate the upload_files to get the filename under the req
		for(size_t it = 0; it < files.size(); it++)
		{
			const binary_file& file = files[it];
			std::cout << "binary file content >> " << file.filename << std::endl;

			//filepath: /home/nsan/Exercises/webserv/sites/tmp/text
			filepath = location->upload_dir + "/" + file.filename;
			std::cout << "filepath: " << filepath << std::endl;

			std::ofstream ofs(filepath.c_str(), std::ios::out | std::ios::binary);

			if (!ofs) {
				std::cerr << "Failed to open file: " << filepath << std::endl;
				if((status = file_check(location->upload_dir, W_OK)) != 200){
					return (status);
				}
			}
			else{
				ofs.write(file.data.c_str(), file.data.size());
				rep._type = "text/html";
				rep._body = "<!DOCTYPE html>\n"
							"<html>\n"
							"<head>\n"
							"<meta charset=\"UTF-8\">\n"
							"<title>Uploaded Files</title>\n"
							"</head>\n"
							"<body>\n"
							"<p>File uploaded successfully</p>\n"
							"</body>\n"
							"</html>";
				}
			req.upload_files().clear();
			ofs.close();
			return (200);
		}
		
	}
	else if(method == "DELETE")
	{
        const std::string &base = location->upload_dir;

        // Must be able to traverse and modify the directory
        status = file_check(base, W_OK | X_OK);
        if (status != 200)
            return status;

        // Basic traversal protection (minimal, but important)
        // remain_path is expected to be like "/file.txt"
        if (remain_path.empty() || remain_path == "/")
            return 400;
        if (remain_path.find("..") != std::string::npos)
            return 403;

        // Build target path safely-ish
        std::string rel = remain_path;
        if (!rel.empty() && rel[0] == '/')
            rel.erase(0, 1);
        const std::string target = base + "/" + rel;

        // Don’t allow deleting directories (optional policy)
        if (is_dir(target))
            return 403;

        if (std::remove(target.c_str()) != 0)
        {
            if (errno == ENOENT)
                return 404;
            if (errno == EACCES || errno == EPERM)
                return 403;
            return 500;
        }

        rep._type = "text/html";
        rep._body =
            "<!DOCTYPE html>\n"
            "<html>\n"
            "<head>\n"
            "<meta charset=\"UTF-8\">\n"
            "</head>\n"
            "<body>\n"
            "<p>File deleted successfully</p>\n"
            "</body>\n"
            "</html>";
        return 200;
    }

    // If method not handled here, signal a proper error (or let caller decide)
    return 405;
}
