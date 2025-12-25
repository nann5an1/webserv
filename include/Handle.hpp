#ifndef IFEATURE_HPP
#define IFEATURE_HPP

#include "Response.hpp"
#include "Request.hpp"
#include "Utils.hpp"

std::string	status_page(int status);

int norm_handle(std::string	&path, Request &req, Response &rep, const t_location* location);

void	redirect_handle(int status, const std::string &path, Response& rep);
// void	handleFileUpload(const t_location* location, Request &req, Response &rep);
int		cgi_handle(std::string &final_path, Request& req, Response& rep);

// void	error_handle();


#endif
