#ifndef IFEATURE_HPP
#define IFEATURE_HPP

#include "Response.hpp"
#include "Request.hpp"
#include "Utils.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include <sstream>

std::string	status_page(int status);

int norm_handle(std::string	&final_path, Request &req, Response &rep, const t_location* location);

void	redirect_handle(int status, const std::string &path, Response& rep);
void	handleFile(const t_location* location, std::string &remain_path, Request &req, Response &rep);


#endif
