#ifndef	CGI_HPP
#define CGI_HPP

#include "IPollable.hpp"
#include "Utils.hpp"
#include "Request.hpp"

enum	cgi_state
{
	CGI_ERROR,
	CGI_WRITING,
	CGI_READING,
	CGI_DONE
};

#define	CGI_CAP	64 * 1024

class	Cgi : public IPollable
{
	private:
		pid_t		_pid;
		int			_in_fd;
		int			_out_fd;
		std::string	_body;
		size_t		_written;
		std::string	_output;

		cgi_state	_state;
		
		Cgi(const Cgi &other);
		Cgi	&operator=(const Cgi &other);
		
		static void	close_fd(int &fd);
	
	public:
		Cgi();
		~Cgi();

		int		execute(std::string& final_path, const std::string* exec_path, Request &req);
		void	handle(uint32_t events);

		bool		done();
		std::string	output();
};

#endif
