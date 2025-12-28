#ifndef HANDLE_HPP
#define HANDLE_HPP

#include <fstream>
#include <iostream>
#include <algorithm>

#include "Response.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Utils.hpp"

typedef struct	s_location	t_location;

std::string	status_page(int status);

int 	norm_handle(std::string	&final_path, Request &req, Response &rep, const t_location* location);
void	redirect_handle(int status, const std::string &path, Response& rep);
int		cgi_handle(std::string &final_path, const t_location *location, Request& req, Response& rep);

void	handleFile(const t_location* location, std::string &remain_path, Request &req, Response &rep);
void	handleFileDelete(const t_location* location, Request &req, Response &rep);
// void	error_handle();


#endif
