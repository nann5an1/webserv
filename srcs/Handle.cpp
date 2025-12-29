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


int	norm_handle(std::string	&final_path, Request &req, Response &rep, const t_location* location)
{
	int	status;
	const std::vector<std::string>	&indexs = location->index_files;
	std::string	path = final_path, index_path;

	// std::cout << "finalPath - path -> " << path << std::endl;
	
	if (is_dir(path))
	{
		for (int i = 0; i < indexs.size(); ++i)
		{
			index_path = path + "/" + indexs[i];
			int status = 0;
			if ((status = file_check(index_path, R_OK)) == 200)
			{
				path = index_path;
				goto response;
			}
		}
		if(location->autoindex){
			if(indexs.empty())
			{ //autoindex is on and index files is empty (list out the files in the directory)
				// location->root
				rep._body = autoIndexOnListing(path);
				rep._type = "text/html";
				if(rep._body.empty())	return (403);
    			return (200);
			}
		}
		else //if auto index is off
			if(indexs.empty()) return (403);
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

// Add this new function to generate file listing HTML
// Improved generate_file_list function with better error handling
std::string generate_file_list(const std::string& upload_dir)
{
    std::string html =
        "<h2>Uploaded Files:</h2>"
        "<script>"
        "function deleteFile(filename) {"
        "  fetch('/upload/' + filename, { method: 'DELETE' })"
        "    .then(res => {"
        "      if (res.ok) location.reload();"
        "      else alert('Delete failed');"
        "    });"
        "}"
        "</script>"
        "<ul>";

    DIR* dir = opendir(upload_dir.c_str());
    if (!dir)
    {
        std::cerr << "Failed to open directory: " << upload_dir << std::endl;
        return html + "<li>Cannot open directory</li></ul>";
    }

    struct dirent* entry;
    int file_count = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Skip directories if you only want files
        std::string full_path = upload_dir + "/" + entry->d_name;
        struct stat st;
        if (stat(full_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
            continue;

        std::string filename(entry->d_name);
        
        std::cout << "Adding file to list: " << filename << std::endl;
        
        html += "<li>";
        html += filename;
        html += " <button onclick=\"deleteFile('" + filename + "')\">Delete</button>";
        html += "</li>";
        
        file_count++;
    }

    closedir(dir);
    
    if (file_count == 0)
        html += "<li>No files uploaded yet</li>";
    
    html += "</ul>";
    
    std::cout << "Total files in listing: " << file_count << std::endl;
    
    return html;
}

/* ====================== add the data from the upload_files of the server into the server's upload_dir ======================*/
int	handleFile(const t_location* location, std::string &remain_path, Request &req, Response &rep){
	std::string filepath;
	std::string method = req.method();

	// std::cout << "method in handleFile" << method << std::endl;
	if(method == "POST")
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
				req.upload_files().clear();
				ofs.close();
				rep._status = 404;
				continue;
			}
			ofs.write(file.data.c_str(), file.data.size());
			req.upload_files().clear();
			ofs.close();
		}
		rep._type = "text/html";
		rep._body = "<!DOCTYPE html>\n"
					"<html>\n"
					"<head>\n"
					"<meta charset=\"UTF-8\">\n"
					"<title>Uploaded Files</title>\n"
					"</head>\n"
					"<body>\n"
					+ generate_file_list(location->upload_dir) +
					"</body>\n"
					"</html>";
	}
	else if(method == "DELETE"){
		std::cout << "remain path <><> " << remain_path << std::endl;
		filepath = location->upload_dir + remain_path;

		if(fileExists(filepath)){ //if file exists in the directory, remove the file
			std::cout << "remain path under file exists again " << remain_path << std::endl;
			std::remove(filepath.c_str());
			// location->upload_dir.erase(location->upload_dir.find(remain_path), remain_path.length());
			std::cout << "filepath removed aldy" << std::endl;
		}
		else
			std::cout << "file does not existed or has been deleted." << std::endl;
	}
	return (200);
}
