#include "Epoll.hpp"
#include "Pollable.hpp"
#include "Utils.hpp"

Epoll::Epoll() : _fd(-1) {}

Epoll::~Epoll()
{
	if (_fd >= 0)
		close(_fd);
}

Epoll&	Epoll::instance()
{
	static Epoll	inst;
	return (inst);
}

int	Epoll::init()
{
	if (_fd >= 0)
		return (0);
	_fd = epoll_create(1);
	return (_fd < 0 ? -1 : 0);
}

int	Epoll::add_ptr(Pollable* poll_obj, uint32_t events)
{
	if (_fd < 0)
		return (-1);
	struct epoll_event	ev;
	std::memset(&ev, 0, sizeof(ev));
	ev.events = events;
	ev.data.ptr = poll_obj;
	return (epoll_ctl(_fd, EPOLL_CTL_ADD, static_cast<int>(*poll_obj), &ev));
}

int	Epoll::mod_ptr(Pollable* poll_obj, uint32_t events)
{
	if (_fd < 0)
		return (-1);
	struct epoll_event	ev;
	std::memset(&ev, 0, sizeof(ev));
	ev.events = events;
	ev.data.ptr = poll_obj;
	return (epoll_ctl(_fd, EPOLL_CTL_MOD, static_cast<int>(*poll_obj), &ev));
}

int	Epoll::del_ptr(Pollable* poll_obj)
{
	if (_fd < 0)
		return	(-1);
	return (epoll_ctl(_fd, EPOLL_CTL_DEL, static_cast<int>(*poll_obj), NULL));
}

int Epoll::wait(struct epoll_event *events, int maxevents, int timeout)
{
	if (_fd < 0)
		return (-1);
	return (epoll_wait(_fd, events, maxevents, timeout));
}

Epoll::operator	fd() const
{
	return (_fd);
}
