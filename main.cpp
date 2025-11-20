#include "Config.hpp"
#include "Webserv.hpp"

int main()
{
	// Config	test("test.conf");
	
	// std::cout << "\n\n\n ______________________" << std::endl;
	// test["server"]["a"].print();
	// std::string	str = test["server"]["server_name"][0];
	// std::cout << "\nserver_name : " << str << std::endl; 
	// std::cout << "key : " << test["server"].obj_begin()->first << std::endl;
	// size_t n = test["server"]["error_page"].size();
	// std::cout << "error_page count: " << n << std::endl;

	// std::cout << typeid(test["server"].obj_begin()->second).name() << std::endl;

	Webserv webserv();
	webserv.fileParser("def.conf");
	// webserv.watchServer();
	return 0;

}