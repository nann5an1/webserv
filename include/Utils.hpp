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

void	init_gphrase();
int		fail(std::string head, int err_no);

template	<typename T>
std::string	to_string(T value)
{
	std::stringstream ss;
	ss << value;
	return (ss.str());
}

#endif
