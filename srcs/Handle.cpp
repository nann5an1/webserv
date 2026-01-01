#include "Handle.hpp"

std::string	status_page(int status)
{
	std::string	status_str = to_string(status), phrase = gphrase[status];
	std::string	content = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n\t<meta charset=\"UTF-8\">\n\t<title>Status Page</title>\n</head>\n<body>\n\t<h1>" + status_str + " " + phrase + "</h1>\n</body>\n</html>";
	return (content);
}

/* ================ READ THE ENTIRE DIRECTORY AND LIST DOWN ================*/
std::string autoIndexOnListing(std::string& path)
{
    DIR* dir = opendir(path.c_str());
    if (!dir)
        return ("");

	std::string html;

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
		html += entry->d_name;

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


int handleServerIndex(Response &rep, const Server *server){
	std::vector<std::string> server_idx = server->server_idx();
	std::string server_path = server->root(), index_path;
	int status;
	bool found_forbidden = false;

	// std::cout << "Server indexes size -> " << server_idx.size() << std::endl; 
	// std::cout << "Server path -> " << server_path << std::endl;
	DIR* dir = opendir(server->root().c_str());

	if(!dir)	throw Error("root directory error >> "+ server->root());

	for (int i = 0; i < server_idx.size(); ++i)
	{
		index_path = server_path + "/" + server_idx[i];
		if ((status = file_check(index_path, R_OK)) == 200)
			if(status == 200){
				status = read_file(index_path, rep._body);
				rep._type = mime_types[get_ext(index_path)];
				return (status);
			}
			if(status == 403)
				found_forbidden = true;
	}
	if(found_forbidden) return (403);
	return (404);
}

int	norm_handle(std::string	&final_path, Request &req, Response &rep,
	 const t_location* location, std::string loc, const Server *server)
{
	int	status;
	const std::vector<std::string>	&indexs = location->index_files;
	std::string	path = final_path, index_path;


	if (is_dir(path)) //directory look out for the indexfiles
	{	
		int prev_code;
		for (int i = 0; i < indexs.size(); ++i)
		{
			index_path = path + "/" + indexs[i];
			std::cout << "index_path " << index_path << std::endl;
			status = 0;
			if ((status = file_check(index_path, R_OK)) == 200)
			{
				path = index_path;
				goto response;
			}
			else if(prev_code == 404 && status == 404) return (403);
			else if (status == 404)
				prev_code = 404;
			
		}
		if(location->autoindex)
		{
			if(indexs.empty())
			{ 
				rep._body = autoIndexOnListing(path);
				rep._type = "text/html";
    			return (200);
			}
		}
		else{ //autoindex is off
			if(!server->server_idx().empty()){ //fallback to the main server's
				if(handleServerIndex(rep, server) == 200){
					rep._type = "text/html";
					rep._status = 200;
					return (200);
				}
				else {
					rep._type = "text/html";
					std::cout << "feel in that going here " << std::endl;
					rep._body = status_page(403);
					return (403);
				}
			}
			else {
				std::cout << "another feeling " << std::endl;
				return (403);
			}
		}
	}
	response:
		status = file_check(path, R_OK);
		if (status == 200)
		{
			status = read_file(path, rep._body);
			rep._type = mime_types[get_ext(path)];
			return (status);
		}
		return (status);
	return (403);
}

void	redirect_handle(int status, const std::string &path, Response& rep)
{
	rep._body = status_page(status);
	rep._type = "text/html";
	rep._status = status;
	
	if (status > 300 && status < 400)
		rep._location = path;
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
		}
		
	}
	else if(method == "DELETE"){
		// std::cout << "remain path <><> " << remain_path << std::endl;
		filepath = location->upload_dir + remain_path;
		int	status = file_check(filepath, R_OK | X_OK);
		if(status == 200){ //if file exists in the directory, remove the file
			std::remove(filepath.c_str());
			rep._type = "text/html";
			rep._body = "<!DOCTYPE html>\n"
					"<html>\n"
					"<head>\n"
					"<meta charset=\"UTF-8\">\n"
					"</head>\n"
					"<body>\n"
					"<p>File delete successfully</p>\n"
					"</body>\n"
					"</html>";
			
			std::cout << "filepath removed aldy" << std::endl;
		}
		else
		{
			std::cout << "file does not exist or has been deleted." << std::endl;
			return (status);
		}
			
	}
	return (200);
}
