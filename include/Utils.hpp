#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstring>
#include <map>
#include <vector>

#include <sstream>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#define	RED	"\033[31m"
#define	RESET "\033[0m"

struct	fd 
{
   	int fd_;
	fd();
    fd(int FD);
    operator int() const;
};

extern std::map<int, const char*>	gphrase;

extern std::map<std::string, std::string> mime_types;

extern const std::string	CRLF;
// extern int   request_category;
// extern std::string cgi_env;
// extern std::string filename;

void	init_global();
bool	is_dir(std::string path);
int		file_check(std::string path, int mod);
int		fail(std::string head, int err_no);
int		read_file(std::string &path, std::string &data);
std::string	get_ext(const std::string& filename);


std::vector<std::string>	split(std::string str, const char delimiter);

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

// For const maps
template <typename MapType, typename KeyType>
const typename MapType::mapped_type* get(const MapType& m, const KeyType& key)
{
    typename MapType::const_iterator it = m.find(key);
    if (it != m.end())
        return &it->second;
    else
        return NULL;  // nullptr doesn't exist in C++98
}

// For non-const maps
template <typename MapType, typename KeyType>
typename MapType::mapped_type* get(MapType& m, const KeyType& key)
{
    typename MapType::iterator it = m.find(key);
    if (it != m.end())
        return &it->second;
    else
        return NULL;
}

#endif
