#ifndef IFEATURE_HPP
#define IFEATURE_HPP

#include "Response.hpp"
#include "Request.hpp"
#include "Utils.hpp"

struct	IFeature
{
	virtual ~IFeature();
	virtual int	handle() = 0;
};

struct	normFeature : IFeature
{
	static int handle(std::string	&path, Request &req, Response &rep, const t_location* location);
};

// struct	errorFeature : IFeature
// {
// 	errorFeature(int file_status, )
// 	int	handle();
// }

#endif
