#include "Utils.hpp"

const std::string	CRLF = "\r\n";

fd::fd() : fd_(-1) {}

fd::fd(int fd_) : fd_(fd_) {}

fd::operator int() const {return (fd_);}


std::map<int, const char*>	gphrase;

void	init_gphrase()
{
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

int	file_check(const char* path, bool dir, int mod)
{
	struct stat st;
	if (stat(path, &st) < 0)
		return (404);
	if (dir && !S_ISDIR(st.st_mode))
		return (403);
	if (access(path, mod) != 0)
		return (403);
	return (200);
}
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

