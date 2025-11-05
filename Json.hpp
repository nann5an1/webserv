#ifndef JSON_HPP
#define JSON_HPP

#include <algorithm>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

class	Json
{
	public:
		enum	Type	{ Null, Object, Array, String, Double, Int, Bool };

	private:
		Type	_type;
		std::string	_str_value;
		double	_db_value;
		int		_int_value;
		bool	_bool_value;

		std::map<std::string, Json>	_obj_value;
		std::vector<Json>	_array_value;

		std::string	trim(std::string &str);

		bool	parse_file(std::ifstream &file);
		bool	parse_value(std::istream &is);

	public:
		Json();
		Json(std::string data);
		Json(double digit);
		Json(int digit);
		Json(bool state);

		Json(const char *filename);
		Json(std::ifstream file);

		Json&	operator[](const std::string &key);
		Json&	operator[](size_t index);

		operator	std::string() const;

		typedef std::vector<Json>::iterator array_iterator;
		typedef std::vector<Json>::const_iterator array_const_iterator;

		typedef std::map<std::string, Json>::iterator object_iterator;
		typedef std::map<std::string, Json>::const_iterator object_const_iterator;

		array_iterator begin();
		array_iterator end();
		array_const_iterator begin() const;
		array_const_iterator end() const;

		object_iterator obj_begin();
		object_iterator obj_end();
		object_const_iterator obj_begin() const;
		object_const_iterator obj_end() const;

		size_t	size() const;
		std::vector<Json>	get_array();
		std::map<std::string, Json>	get_obj();
		void	print();
};


#endif
