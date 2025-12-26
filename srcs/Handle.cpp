#include "Handle.hpp"

std::string	status_page(int status)
{
	std::string	status_str = to_string(status), phrase = gphrase[status];
	std::string	content = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n\t<meta charset=\"UTF-8\">\n\t<title>Status Page</title>\n</head>\n<body>\n\t<h1>" + status_str + " " + phrase + "</h1>\n</body>\n</html>";
	return (content);
}

int	norm_handle(std::string	&final_path, Request &req, Response &rep, const t_location* location)
{
	int	status;
	const std::vector<std::string>	&indexs = location->index_files;
	std::string	path = final_path;

	if (is_dir(path))
	{
		for (int i = 0; i < indexs.size(); ++i)
		{
			if (file_check(path + "/" + indexs[i], R_OK) == 200)
			{
				path += "/" + indexs[i];
				goto response;
			}
		}
		std::cout << "PATH >> " << path << std::endl;
		std::cout << "autoindex >> " << location->autoindex << std::endl;
		if (location->autoindex)
		{
			std::cout << YELLOW << "autoindex is on " << RESET << std::endl;
			return (200);
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


bool fileExists(std::string &path){
	std::cout << "getter path >> " << path << std::endl;
	std::ifstream file(path.c_str());
	return file.good();
}

/* ====================== add the data from the upload_files of the server into the server's upload_dir ======================*/
void	handleFile(const t_location* location, std::string &remain_path, Request &req, Response &rep){
	std::cout << "Server's location upload dir >> " <<  location->upload_dir << std::endl;
	std::string filepath;
	std::string method = req.method();

	if(method == "POST"){ //METHOD = POST
		std::vector<binary_file> files = req.upload_files();
		// std::cout << "files size DEBUG >> " << files.size() << std::endl;

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
			ofs.close();
		}
	}
	else if(method == "DELETE"){
		std::cout << "remain path <><> " << remain_path << std::endl;
		filepath = location->upload_dir + remain_path;
		std::cout << "DELETE FILEPATH >> " << filepath << std::endl;
		if(fileExists(filepath)){ //if file exists in the directory, remove the file
			std::remove(filepath.c_str());
			std::cout << "filepath removed aldy" << std::endl;
		}
	}
		

}



// void	cgi_handle()
// {
	
// }

// void	error_handle()
// {

// }