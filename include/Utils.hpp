#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstring>
#include <map>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <errno.h>

#define	RED	"\033[31m"
#define	RESET "\033[0m"

extern std::map<int, const char*>	gphrase;

extern const std::string	CRLF;
// extern int   request_category;
// extern std::string cgi_env;
// extern std::string filename;

void	init_gphrase();
int		fail(std::string head, int err_no);

template	<typename T>
std::string	to_string(T value)
{
	std::stringstream ss;
	ss << value;
	return (ss.str());
}

// Template function to print any map
template <typename K, typename V>
void	printMap(const std::map<K, V> &m) {
    for (typename std::map<K, V>::const_iterator it = m.begin(); it != m.end(); ++it) {
        std::cout << it->first << " -> " << it->second << std::endl;
    }
}

#endif
