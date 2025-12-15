#include "IFeature.hpp"

int	normFeature::handle(std::string	&path, Request &req, Response &rep, const t_location* location)
{
	std::string	*type;

	if	(!is_dir(path))
	{
		int status = file_check(path, R_OK);
		if (status == 200)
		{
			status = read_file(path, rep._body);
			
		}
		else	
			return (status);
	}
	else
	{
		get(location, final)
	}
	// else
	// {
	// 	location
	// }

	std::cout << "normal: " << status << std::endl;
			std::cout << "req.ext(): " << req.ext() << std::endl;
			type = get(mime_types, req.ext());
			if (!type)
				*type = std::string("text/plain");
			rep._type = *type;
}