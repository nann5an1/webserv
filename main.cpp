#include "Json.hpp"

int main()
{
	Json	test("test_file");
	test.print();

	// std::string	str = test["server"]["server_name"][0];
	// std::cout << "server_name : " << str << std::endl; 
	// std::cout << "key : " << test["server"].obj_begin()->first << std::endl;
}