#ifndef EPOLL_HPP
#define EPOLL_HPP

#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include "Utils.hpp"

struct IPollable; // forward

struct Epoll
{
	private:
		int	_fd;
		Epoll();
		~Epoll();
	public:
		static Epoll& instance();

		int init();

		operator	fd() const;

		int add_fd(IPollable* poll_obj, fd fd_, uint32_t events);
		int mod_fd(IPollable* poll_obj, fd fd_, uint32_t events);
		int del_fd(fd fd_);

		int wait(struct epoll_event *events, int maxevents, int timeout);
};

#endif