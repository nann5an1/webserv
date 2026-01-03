#include "Config.hpp"

Config::Config()	:	_type(Null) {}

Config::Config(std::string content)	:	_type(String), _str_value(content) {}

Config::Config(const char *filename)
{
	std::ifstream	file(filename);
	parse_file(file);
}
Config::Config(std::ifstream file)
{
	parse_file(file);
}

std::string	Config::trim(std::string &str)
{
	const char *space = " \t\r\n";
	std::size_t	start = str.find_first_not_of(space);
	std::size_t	end = str.find_last_not_of(space);
	if (start == end)
		return ("");
	return (str = str.substr(start, end - start + 1));
}

Config& Config::operator[](const std::string &key) 
{
	if (_type != Object) {
		_type = Object;
		_obj_value.clear();
	}
	return _obj_value[key];
}

Config& Config::operator[](size_t index) 
{
	if (_type != Array) {
		_type = Array;
		_array_value.clear();
	}
	if (index >= _array_value.size())
		_array_value.resize(index + 1);
	return _array_value[index];
}

Config::operator std::string() const
{
	if (_type != String)
		throw std::runtime_error("Json: type is not String");
	return _str_value;
}

Config::array_iterator Config::begin() { return _array_value.begin(); }
Config::array_iterator Config::end()   { return _array_value.end(); }
Config::array_const_iterator Config::begin() const { return _array_value.begin(); }
Config::array_const_iterator Config::end()   const { return _array_value.end(); }

Config::object_iterator Config::obj_begin() { return _obj_value.begin(); }
Config::object_iterator Config::obj_end()   { return _obj_value.end(); }
Config::object_const_iterator Config::obj_begin() const { return _obj_value.begin(); }
Config::object_const_iterator Config::obj_end()   const { return _obj_value.end(); }

size_t	Config::size() const
{
	switch (_type)
	{
		case Array:
			return (_array_value.size());
		case Object:
			return (_obj_value.size());
		case String:
			return (_str_value.size());
		default :
			return (0);
	}
}

std::vector<Config>	Config::get_array()
{
	if (_type == Array)
		return (_array_value);
	else
		return (std::vector<Config>());
}

std::map<std::string, Config>	Config::get_obj()
{
	if (_type == Object)
		return (_obj_value);
	else
		return (std::map<std::string, Config>());
}

bool	Config::parse_file(std::ifstream &file)
{
	_type = Object;
	return parse_value(file);
}


bool Config::parse_value(std::istream &is)
{
    std::string line;
    std::string key;
    
    while (std::getline(is, line))
    {
        std::istringstream iss(line);
        std::string token;

        while (iss >> token)
        {
            if (token[0] == '#')
                break;
            if (token == "}")
           		return true;
			if (token != "{" && token.back() != '{' && key.empty())
				key = token;
			if (token.back() == '{')
			{
				token.pop_back();
				if (key.empty())
					key = token;
				Config	value;
				value._type = Object;
				value.parse_value(is);
				_obj_value[key] = value;
				key.clear();
				continue;
			}
			if (_type == Object)
			{
				Config	value;
				std::string	sub_token;

				value._type = Array;
				while (iss >> sub_token)
				{
					Config	arr_val;
					arr_val._type = Null;
					arr_val.parse_value(is);
					arr_val.print();
					value._array_value.push_back(arr_val);
				}
				_obj_value[key] = value;
				continue;
			}
			if (_type == Null)
			{
				Config	value;
				value._type = String;
				value._str_value = token;
				continue;
			}
		}
	}
	return (true);
}

void	Config::print()
{
	switch (_type)
	{
		case	Config::Null:
			break;
		case	Config::String:
			break;
		case	Config::Array:
			std::cout << "[";
			for (size_t i = 0; i < _array_value.size(); ++i)
			{
				_array_value[i].print();
				if (i + 1 < _array_value.size())
					std::cout << ", ";
			}
			std::cout << "]";
			break;
		case	Config::Object:
			for (std::map<std::string, Config>::iterator it = _obj_value.begin(); it != _obj_value.end(); ++it)
			{
				it->second.print();
				std::map<std::string, Config>::iterator next = it;
				++next;
			}
			break;
	}
}