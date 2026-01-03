#ifndef UTILS_HPP
#define UTILS_HPP

#include <map>
#include <vector>

#include <algorithm>
#include <exception>
#include <iostream>
#include <iterator>
#include <sstream>

#include <cstring>

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdint.h>

#define	RED	"\033[31m"
#define YELLOW "\033[33m"
#define	RESET "\033[0m"

#define UNKNOWN 0
#define GET (1 << 0)
#define POST (1 << 1)
#define DELETE (1 << 2)

extern volatile sig_atomic_t g_stop;

struct	fd 
{
	int	fd_;
	fd();
	fd(int FD);
	operator	int() const;
};

int	pipe(fd fds[2]);

extern std::map<int, const char*>	gphrase;

extern std::map<std::string, std::string> mime_types;

extern const std::string	CRLF;
// extern int   request_category;
// extern std::string cgi_env;
// extern std::string filename;

void	init_global();
void	install_signals();
void	on_signal(int signo);
bool	is_dir(std::string path);
int		file_check(std::string path, int mod);
int		fail(std::string head, int err_no);
int		read_file(std::string &path, std::string &data);
int     identify_method(const std::string& method);
int		identify_method(const char *method);
bool	set_nblocking(fd fd_);

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
	std::cout << "key " << key << std::endl;
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

template <typename T>
void print_container(const T& container)
{
    typedef typename T::value_type value_type;
    typedef typename T::const_iterator const_iterator;

    for (const_iterator it = container.begin(); it != container.end(); ++it)
    {
        if (*it != (value_type)NULL) // skip NULLs
        {
            std::cout << *it << " ";
        }
    }
    std::cout << std::endl;
}

template <typename MapType>
void	print_map(const MapType& container)
{
	for (typename MapType::const_iterator ci = container.begin(); ci != container.end(); ++ci)
		std::cout << ci->first << " " << ci->second << ", ";
	std::cout << std::endl;
}
class Error : public std::runtime_error 
{
	public:
		Error();
		Error(const std::string &msg);
};

#endif

