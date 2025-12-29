#ifndef EPOLL_HPP
#define EPOLL_HPP

#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>

#include <sys/epoll.h>

#include "Utils.hpp"

struct Pollable; // forward

struct Epoll
{
	private:
		fd	_fd;
		Epoll();
		~Epoll();
	public:
		static Epoll& instance();

		int init();

		operator	fd() const;

		int add_ptr(Pollable* p, uint32_t events);
		int mod_ptr(Pollable* p, uint32_t events);
		int del_ptr(Pollable* p);

		int wait(struct epoll_event *events, int maxevents, int timeout);
};

#endif