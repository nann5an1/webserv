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
	CGI_KILL,
	CGI_DONE,
};

#define	CGI_CAP	64 * 1024
#define	CGI_TIMEOUT 10


class	Cgi : public IPollable
{
	private:
		pid_t		_pid;
		int			_in_fd;
		int			_out_fd;
		std::string	_body;
		size_t		_written;
		std::string	_output;

		std::time_t	_time;

		cgi_state	_state;
		
		Cgi(const Cgi &other);
		Cgi	&operator=(const Cgi &other);
		
		static void	close_fd(int &fd); 	
	
	public:
		Cgi();
		~Cgi();

		int		execute(std::string& final_path, const std::string* exec_path, Request &req);
		void	handle(uint32_t events);

		std::string	output();

		std::time_t	get_time() const;

		bool	is_timeout() const;
		void	timeout();

		cgi_state	state() const;
};

#endif
