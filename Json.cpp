#include "Json.hpp"

Json::Json()	:	_type(Null) {}

Json::Json(std::string content)	:	_type(String), _str_value(content) {}

Json::Json(double digit) : _type(Double), _db_value(digit) {}

Json::Json(int digit) : _type(Int), _int_value(digit) {}

Json::Json(bool state) : _type(Bool), _bool_value(state) {}

Json::Json(const char *filename)
{
	std::ifstream	file(filename);
	parse_file(file);
}
Json::Json(std::ifstream file)
{
	parse_file(file);
}

std::string	Json::trim(std::string &str)
{
	const char *space = " \t\r\n";
	std::size_t	start = str.find_first_not_of(space);
	std::size_t	end = str.find_last_not_of(space);
	return (str = str.substr(start, end - start + 1));
}

Json& Json::operator[](const std::string &key) 
{
	if (_type != Object) {
		_type = Object;
		_obj_value.clear();
	}
	return _obj_value[key];
}

Json& Json::operator[](size_t index) 
{
	if (_type != Array) {
		_type = Array;
		_array_value.clear();
	}
	if (index >= _array_value.size())
		_array_value.resize(index + 1);
	return _array_value[index];
}

Json::operator std::string() const
{
    if (_type != String)
        throw std::runtime_error("Json: type is not String");
    return _str_value;
}

Json::array_iterator Json::begin() { return _array_value.begin(); }
Json::array_iterator Json::end()   { return _array_value.end(); }
Json::array_const_iterator Json::begin() const { return _array_value.begin(); }
Json::array_const_iterator Json::end()   const { return _array_value.end(); }

Json::object_iterator Json::obj_begin() { return _obj_value.begin(); }
Json::object_iterator Json::obj_end()   { return _obj_value.end(); }
Json::object_const_iterator Json::obj_begin() const { return _obj_value.begin(); }
Json::object_const_iterator Json::obj_end()   const { return _obj_value.end(); }

bool	Json::parse_file(std::ifstream &file)
{
	_type = Object;
	return parse_value(file);
}

bool Json::parse_value(std::istream &is)
{
	std::string line;
	std::string pending_key;

	while (std::getline(is, line))
	{
		trim(line);
		if (line.empty() || line[0] == '#')
			continue;

		if (line == "}")
			return true;

		std::istringstream iss(line);
		std::string key, token;
		iss >> key;

		// skip empty
		if (key.empty())
			continue;

		// handle case like "server" followed by "{" on next line
		if (key == "{")
		{
			if (pending_key.empty())
				continue; // malformed block
			Json value;
			value._type = Object;
			value.parse_value(is);
			_obj_value[pending_key] = value;
			pending_key.clear();
			continue;
		}

		if (!(iss >> token))
		{
			// line only had one token — could be key waiting for '{'
			pending_key = key;
			continue;
		}

		Json	value;
		// normal case: key + value(s)
		if (token == "{")
		{
			value._type = Object;
			value.parse_value(is);
		}
		else
		{
			value._type = Array;
			do
			{
                if (!token.empty() && token[token.size() - 1] == ';')
                    token = token.substr(0, token.size() - 1);
                if (!token.empty())
                    value._array_value.push_back(Json(token));

            } while (iss >> token);
        }
		_obj_value[key] = value;
	}
	return true;
}

void	Json::print()
{
	switch (_type)
	{
		case	Json::Null:
			std::cout << "null";
			break;
		case	Json::Int:
			std::cout << _int_value;
			break;
		case	Json::Double:
			std::cout << _db_value;
			break;
		case	Json::String:
			std::cout << _str_value;
			break;
		case	Json::Bool:
			std::cout << (_bool_value ? "true" : "false");
			break;
		case	Json::Array:
			std::cout << "[";
			for (size_t i = 0; i < _array_value.size(); ++i)
			{
				_array_value[i].print();
				if (i + 1 < _array_value.size())
					std::cout << ", ";
			}
			std::cout << "]";
			break;
		case	Json::Object:
			for (std::map<std::string, Json>::iterator it = _obj_value.begin(); it != _obj_value.end(); ++it)
			{
				std::cout << it->first << " : ";
				it->second.print();
				std::cout << "\n";
				std::map<std::string, Json>::iterator next = it;
				++next;
			}
			break;
	}
}