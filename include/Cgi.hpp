#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include <stdint.h>
#include <sys/types.h>

#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"

enum cgi_state
{
	CGI_CREATED,
	CGI_WRITING,
	CGI_READING,
	CGI_DONE,
	CGI_ERROR
};

class	Cgi	: public Pollable
{
	private:
		pid_t		_pid;
		int			_in_fd;
		int			_out_fd;
		cgi_state	_state;

		size_t		_written;
		std::string	_output;
		std::string	_body;

	public:
		Cgi();
		Cgi(const Cgi &other);
		Cgi &operator=(const Cgi &other);
		~Cgi();

		int execute(std::string &final_path, const t_location *location, Request& req, Response& rep);

		void handle(uint32_t events);

		bool done()   const;
		bool error()  const;

		int  in_fd()  const;
		int  out_fd() const;

		const std::string &output() const;
};

#endif
