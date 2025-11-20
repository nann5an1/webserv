#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <algorithm>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

class	Config
{
	public:
		enum	Type	{ Null, Object, Array, String};

	private:
		Type	_type;

		std::string	_str_value;
		std::map<std::string, Config>	_obj_value;
		std::vector<Config>	_array_value;

		std::string	trim(std::string &str);

		bool	parse_file(std::ifstream &file);
		bool	parse_value(std::istream &is);

	public:
		Config();
		Config(std::string data);

		Config(const char *filename);
		Config(std::ifstream file);

		Config&	operator[](const std::string &key);
		Config&	operator[](size_t index);

		operator	std::string() const;

		typedef std::vector<Config>::iterator array_iterator;
		typedef std::vector<Config>::const_iterator array_const_iterator;

		typedef std::map<std::string, Config>::iterator object_iterator;
		typedef std::map<std::string, Config>::const_iterator object_const_iterator;

		array_iterator begin();
		array_iterator end();
		array_const_iterator begin() const;
		array_const_iterator end() const;

		object_iterator obj_begin();
		object_iterator obj_end();
		object_const_iterator obj_begin() const;
		object_const_iterator obj_end() const;

		size_t	size() const;
		std::vector<Config>	get_array();
		std::map<std::string, Config>	get_obj();
		void	print();
};


#endif
