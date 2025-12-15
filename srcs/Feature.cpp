#include "IFeature.hpp"

int	normFeature::handle(std::string	&path, Request &req, Response &rep, const t_location* location)
{
	int	status;
	std::string	*type;
	const std::vector<std::string>	&indexs = location->index_files;

	if (is_dir(path))
	{
		if (indexs.empty() && location->autoindex)
			return (std::cout << "autoindex" << std::endl, 200);
		for (int i = 0; i < indexs.size(); ++i)
		{
			if (file_check(path + "/" + indexs[i], R_OK) == 200)
			{
				path += "/" + indexs[i];
				goto response;
			}
		}
		return (403);
	}
	response:

		status = file_check(path, R_OK);
		if (status == 200)
		{
			status = read_file(path, rep._body);
			rep._type = "text/" + get_ext(path);
			return (status);
		}
		return (status);
	// status = file_check(path, R_OK);
}