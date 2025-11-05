#include "Json.hpp"

int main()
{
	Json	test("test.conf");
	// test.print();

	std::string	str = test["server"]["server_name"][0];
	std::cout << "\nserver_name : " << str << std::endl; 
	std::cout << "key : " << test["server"].obj_begin()->first << std::endl;
	size_t n = test["server"]["error_page"].size();
	std::cout << "error_page count: " << n << std::endl;

	std::cout << typeid(test["server"].obj_begin()->second).name() << std::endl;

}