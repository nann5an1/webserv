#include "Epoll.hpp"
#include "IPollable.hpp"
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

int	Epoll::add_fd(IPollable* poll_obj, int fd_, uint32_t events)
{
	if (_fd < 0 || fd_ < 0)
		return (-1);
	struct epoll_event	ev;
	std::memset(&ev, 0, sizeof(ev));
	ev.events = events;
	ev.data.ptr = poll_obj;
	return (epoll_ctl(_fd, EPOLL_CTL_ADD, fd_, &ev));
}

int Epoll::mod_fd(IPollable* poll_obj, int fd_, uint32_t events)
{
	if (_fd < 0 || fd_ < 0)
		return (-1);
	struct epoll_event	ev;
	std::memset(&ev, 0, sizeof(ev));
	ev.events = events;
	ev.data.ptr = poll_obj;
	return (epoll_ctl(_fd, EPOLL_CTL_MOD, fd_, &ev));

}

int Epoll::del_fd(int fd_)
{
	if (_fd < 0 || fd_ < 0)
		return (-1);
	return (epoll_ctl(_fd, EPOLL_CTL_DEL, fd_, NULL));
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
