#include "Webserv.hpp"
#include "Server.hpp"
#include "Request.hpp"


int main(int ac, char **av)
{
    if(ac > 2) return 1;

    Webserv webserv;
    webserv.fileParser(av[1]);
    // webserv.print_server_head();
    // webserv.start();

	// const char* raw_request = 
    //     "POST /upload HTTP/1.1\n"
    //     "Host: localhost:8080\n"
    //     "Content-Type: application/json\n" 
    //     "Content-Length: 27\n"                     
    //     "Connection: keep-alive\n";

    // const char* raw_request2 =
    //     "GET /script.pl?name=john&age=20 HTTP/1.1\n"
    //     "Host: localhost:8080\n"
    //     "Accept: text/html\n"
    //     "Connection: close";

    // const char* raw_request3 = 
    // "POST /script.php HTTP/1.1\n"
    // "Host: localhost:8080\n"
    // "Content-Type: application/x-www-form-urlencoded\n"
    // "Content-Length: 19\n"

    // "name=john&age=20";

//    const char* raw_request4 =
//     "POST /upload HTTP/1.1\r\n"
//     "Host: localhost:8080\r\n"
//     "Content-Type: multipart/form-data; boundary=----ABC123\r\n"
//     "Content-Length: 514\r\n"
//     "\r\n"
//     "------ABC123\r\n"
//     "Content-Disposition: form-data; name=\"file\"; filename=\"test.jpg\"\r\n"
//     "Content-Type: image/jpeg\r\n"
//     "\r\n"
//     "\xFF\xD8\xFF\xE0\x00\x10\x4A\x46\x49\x46\x00\x01"
//     "hello world"
//     "\xFF\xD9"
//     "\r\n"
//     "------ABC123--\r\n";

     const char raw_request[] =
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Content-Type: multipart/form-data; boundary=----XYZ789\r\n"
        "Content-Length: 500\r\n"
        "\r\n"
        "------XYZ789\r\n"
        "Content-Disposition: form-data; name=\"file1\"; filename=\"test.jpg\"\r\n"
        "Content-Type: image/jpeg\r\n"
        "\r\n"
        "\xFF\xD8\xFF\xE0\x00\x10\x4A\x46\x49\x46\x00\x01"   // fake JPEG header
        "HelloWorld1"
        "\xFF\xD9"
        "\r\n"
        "------XYZ789\r\n"
        "Content-Disposition: form-data; name=\"file2\"; filename=\"example.bin\"\r\n"
        "Content-Type: application/octet-stream\r\n"
        "\r\n"
        "\xDE\xAD\xBE\xEF\x00\x01\x02\x03"   // fake binary
        "HelloWorld2"
        "\xAA\xBB\xCC\xDD"
        "\r\n"
        "------XYZ789--\r\n";

    Request req;
    req.parseRequest(raw_request);

    // if (req.parseRequest(raw_request)) {
    //     std::cout << "Method: " << req.getMethod() << "\n";
    //     std::cout << "Path: " << req.getPath() << "\n";
    //     std::cout << "Query: " << req.getQuery() << "\n";
    //     std::cout << "HTTP Version: " << req.getHttpVersion() << "\n";
    //     std::cout << "Host: " << req.getHostname() << "\n";
    //     std::cout << "Port: " << req.getPort() << "\n";
    //     std::cout << "Connection: " << req.getConnStatus() << "\n";
    // }
    return 0;
    // Server	server;
	// int		status;
	// if ((status = server.start()) != 0)
	// 	std::cerr << RED << "Error: Socket: " << strerror(status) << RESET << std::endl;

	// fd	sock = server;
	// std::cout << "port: " << server << std::endl;
	// std::cout << "sock: " << sock << std::endl;
	// std::cout << "id: " << (std::string)server << std::endl;
}