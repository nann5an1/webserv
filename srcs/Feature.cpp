#include "IFeature.hpp"

int	normFeature::handle(std::string	&path, Request &req, Response &rep, const t_location* location)
{
	std::string	*type;

	if	(!is_dir(path))
	{
		int status = file_check(path, R_OK);
		if (status == 200)
		{
			rep._status = read_file(path, rep._body);
			type = get(mime_types, req.ext());
			if (!type)
				*type = std::string("text/plain");
			rep._type = *type;
		}
		else	
			return (status);
	}
	return (200);
	// else
	// {
	// 	location
	// }
}