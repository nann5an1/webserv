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
        return "";

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
	dirent	*entry;
	int	status;
	const std::vector<std::string>	&indexs = location->index_files;
	std::string	path = final_path;
	DIR* dir;
	std::string html, index_path;

	// std::cout << "finalPath - path -> " << path << std::endl;
	
	if (is_dir(path))
	{
		for (int i = 0; i < indexs.size(); ++i)
		{
			index_path = path + "/" + indexs[i];
			int status = 0;
			// std::cout << "path and file combined >> " << index_path << std::endl;
			if ((status = file_check(index_path, R_OK)) == 200)
			{
				path = index_path;
				goto response;
			}
			std::cout << "status: " << status << std::endl;
		}
		if(location->autoindex){
			if(indexs.empty()){ //autoindex is on and index files is empty (list out the files in the directory)
				rep._body = autoIndexOnListing(path);
				rep._type = "text/html";
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


bool fileExists(std::string &path){
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
void	handleFile(const t_location* location, std::string &remain_path, Request &req, Response &rep){
	std::cout << "Server's location upload dir >> " <<  location->upload_dir << std::endl;
	std::string filepath;
	std::string method = req.method();

	std::cout << "method in handleFile" << method << std::endl;
	if(method == "POST"){ //METHOD = POST
		std::vector<binary_file> files = req.upload_files();
		std::cout << "files size DEBUG >> " << files.size() << std::endl;
		
		//iterate the upload_files to get the filename under the req
		for(size_t it = 0; it < files.size(); it++){
			const binary_file& file = files[it];
			std::cout << "binary file content >> " << file.filename << std::endl;

			//filepath: /home/nsan/Exercises/webserv/sites/tmp/text
			filepath = location->upload_dir + "/" + file.filename;
			std::cout << "filepath: " << filepath << std::endl;
			std::ofstream ofs(filepath.c_str(), std::ios::out | std::ios::binary);

			if (!ofs) {
				std::cerr << "Failed to open file: " << filepath << std::endl;
				continue;
			}
			ofs.write(file.data.c_str(), file.data.size());
			// req.upload_files().erase(req.upload_files().begin() + it);
			req.upload_files().clear();
			ofs.close();

		}
		rep._status = 200;
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
		std::cout << "DELETE FILEPATH >> " << filepath << std::endl;
		if(fileExists(filepath)){ //if file exists in the directory, remove the file
			std::remove(filepath.c_str());
			std::cout << "filepath removed aldy" << std::endl;
		}
		else{
			std::cout << "file does not existed or has been deleted." << std::endl;
		}
	}
}
