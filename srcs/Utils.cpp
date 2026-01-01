#include "Utils.hpp"

const std::string	CRLF = "\r\n";

fd::fd() : fd_(-1) {}

fd::fd(int fd_) : fd_(fd_) {}

fd::operator int() const {return (fd_);}

int	pipe(fd fds[2])
{
	int	tmp[2];
	if (::pipe(tmp) == -1)
		return (-1);
	fds[0] = fd(tmp[0]);
	fds[1] = fd(tmp[1]);
	return (0);
}

std::map<int, const char*>	gphrase;

std::map<std::string, std::string> mime_types;

void	init_global()
{
	mime_types["html"] = "text/html";
    mime_types["htm"]  = "text/html";
    mime_types["css"]  = "text/css";
    mime_types["js"]   = "application/javascript";
    mime_types["json"] = "application/json";
    mime_types["txt"]  = "text/plain";
    mime_types["png"]  = "image/png";
    mime_types["jpg"]  = "image/jpeg";
    mime_types["jpeg"] = "image/jpeg";
    mime_types["gif"]  = "image/gif";
    mime_types["ico"]  = "image/x-icon";
    mime_types["pdf"]  = "application/pdf";
    mime_types["svg"]  = "image/svg+xml";
    mime_types["xml"]  = "application/xml";
    mime_types["zip"]  = "application/zip";

	// 0xx Not found
	gphrase[0] = "";

	// 1xx Informational
	gphrase[100] = "Continue";
	gphrase[101] = "Switching Protocol";
	gphrase[102] = "Processing";

    // 2xx Success
	gphrase[200] = "OK";
	gphrase[201] = "Created";
	gphrase[202] = "Accepted";
	gphrase[204] = "No Content";
	gphrase[206] = "Partial Content";

	// 3xx Redirection
	gphrase[300] = "Multiple Choices";
	gphrase[301] = "Moved Permanently";
	gphrase[302] = "Found";
	gphrase[303] = "See Other";
	gphrase[304] = "Not Modified";
	gphrase[307] = "Temporary Redirect";
	gphrase[308] = "Permanent Redirect";

	// 4xx Client errors
	gphrase[400] = "Bad Request";
	gphrase[401] = "Unauthorized";
	gphrase[403] = "Forbidden";
	gphrase[404] = "Not Found";
	gphrase[405] = "Method Not Allowed";
	gphrase[408] = "Request Timeout";
	gphrase[409] = "Conflict";
	gphrase[411] = "Length Required";
	gphrase[413] = "Payload Too Large";
	gphrase[414] = "URI Too Long";
	gphrase[415] = "Unsupported Media Type";
	gphrase[418] = "I'm a teapot"; // optional
	gphrase[429] = "Too Many Requests";

	// 5xx Server errors
	gphrase[500] = "Internal Server Error";
	gphrase[501] = "Not Implemented";
	gphrase[502] = "Bad Gateway";
	gphrase[503] = "Service Unavailable";
	gphrase[504] = "Gateway Timeout";
	gphrase[505] = "HTTP Version Not Supported";
}

int	fail(std::string head, int err_no)
{
	std::cerr << RED << "[error]\t\t" << head;
	if (err_no > 0)
		std::cerr << ": " << strerror(err_no);
	std::cerr << RESET << std::endl;
	return (err_no);
}

int read_file(std::string &path, std::string &data)
{
	int	fd = open(path.c_str(), O_RDONLY);
	if (fd < 0)
	{
		
		fail("File: " + path, errno);
		if (errno == ENOENT)
			return (404);
		if (errno == EACCES)
			return (403);
			
		return (500); //internal server error
	}
	
	char	buffer[4096];
	size_t n;
	while ((n = read(fd, buffer, sizeof(buffer))) > 0)
		data.append(buffer, n);
	if (n < 0)
	{
		data.clear();
		fail("File: " + path, errno);

		if(errno == EACCES){
			std::cout << "ERROR 403 RETURNED" << std::endl;
			return (403);
		}
		return (500);
	}
	close(fd);
	return (200);
}

std::vector<std::string>	split(std::string str, const char delimiter)
{
	std::vector<std::string> result;
    
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        result.push_back(token);
    }
	return (result);
}

bool	is_dir(std::string path)
{
	struct stat	st;

	if (stat(path.c_str(), &st) == 0)
	 	return (S_ISDIR(st.st_mode));
	return (false);
}

int	file_check(std::string path, int mod)
{
	struct stat st;
	const char *loc = path.c_str();
	if (stat(loc, &st) < 0)
	{
		std::cout << "hi hi 404" << std::endl;
		return (404);
	}
	if (access(loc, mod) != 0)
	{
		std::cout << "hi hi 403" << std::endl;
		return (403);
	}
	return (200);
}

std::string	get_ext(const std::string& filename)
{
    if (filename.empty())
        return "";

    std::string::size_type dot_pos = filename.find_last_of('.');
    std::string::size_type slash_pos = filename.find_last_of("/\\");

    // No dot, or dot is part of a directory name
    if (dot_pos == std::string::npos ||
        (slash_pos != std::string::npos && dot_pos < slash_pos))
        return "";

    return filename.substr(dot_pos + 1);
}

int	identify_method(const char *method)
{
	std::string	mth(method);
	return (identify_method(mth));
}

int	identify_method(const std::string& method)
{
	if (method == "GET")
		return GET;
	else if (method == "POST")
		return POST;
	else if (method == "DELETE")
		return DELETE;
	return (UNKNOWN);
}

bool	set_nblocking(fd fd_)
{
	int flags = fcntl(fd_, F_GETFL, 0);
    if (flags == -1)
		return (false);
    return (fcntl(fd_, F_SETFL, flags | O_NONBLOCK) != -1);
}

Error::Error() : std::runtime_error(std::string(RED) + "Error: " + RESET) {}

Error::Error(const std::string &msg) : std::runtime_error(std::string(RED) + "Error: " + msg + RESET) {}

// bool fileExists(const char* path) {
//     struct stat st;
//     return (stat(path, &st) == 0);
// }

// bool isDirectory(const char* path) {
//     struct stat st;
//     if (stat(path, &st) != 0) return false;
//     return S_ISDIR(st.st_mode);
// }

// bool canRead(const char* path) {
//     return (access(path, R_OK) == 0);
// }

// bool canWrite(const char* path) {
//     return (access(path, W_OK) == 0);
// }

// bool canExecute(const char* path) {
//     return (access(path, X_OK) == 0);
// }

