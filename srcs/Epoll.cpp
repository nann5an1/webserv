#include "Epoll.hpp"
#include "IPollable.hpp"
#include "Utils.hpp"

Epoll::Epoll() : _fd(-1) {}

Epoll::~Epoll()
{
	std::set<IPollable*>	destory;

	std::cout << "got called" << std::endl;
	for (std::map<fd, IPollable*>::iterator it = _objs.begin(); it != _objs.end(); ++it)
	{
		IPollable* obj = it->second;
		if (obj)
			destory.insert(obj);
	}
	for (std::set<IPollable*>::iterator it = destory.begin(); it != destory.end(); ++it)
	{
		IPollable* obj = *it;
		if (obj)
			obj->cleanup();
	}
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
	if (_fd < 0)
		return (-1);
    int flags = fcntl(_fd, F_GETFL, 0);
	if (flags == -1)
	{
		close(_fd);
		_fd = -1;
		return (-1);
	}
	if (fcntl(_fd, F_SETFL, flags | FD_CLOEXEC) == -1)
	{
		close(_fd);
		_fd = -1;
		return (-1);
	}
	return (0);
}

int	Epoll::add_fd(IPollable* poll_obj, fd fd_, uint32_t events)
{
	if (_fd < 0 || fd_ < 0)
		return (-1);
	struct epoll_event	ev;
	std::memset(&ev, 0, sizeof(ev));
	ev.events = events;
	ev.data.ptr = poll_obj;
	int status = epoll_ctl(_fd, EPOLL_CTL_ADD, fd_, &ev);
	if (status != -1)
		_objs[fd_] = poll_obj;
	return (status);
}

int Epoll::mod_fd(IPollable* poll_obj, fd fd_, uint32_t events)
{
	if (_fd < 0 || fd_ < 0)
		return (-1);
	struct epoll_event	ev;
	std::memset(&ev, 0, sizeof(ev));
	ev.events = events;
	ev.data.ptr = poll_obj;
	return (epoll_ctl(_fd, EPOLL_CTL_MOD, fd_, &ev));

}

int Epoll::del_fd(fd fd_)
{
	if (_fd < 0 || fd_ < 0)
		return (-1);
	int status = epoll_ctl(_fd, EPOLL_CTL_DEL, fd_, NULL);
	if (status != -1)
		_objs.erase(fd_);
	return (status);
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

void Epoll::objs_timeout()
{
	std::set<IPollable*>	timed_out;

	for (std::map<fd, IPollable*>::iterator it = _objs.begin(); it != _objs.end(); ++it)
	{
		IPollable* obj = it->second;
		if (obj && obj->is_timeout())
			timed_out.insert(obj);
	}
	for (std::set<IPollable*>::iterator it = timed_out.begin(); it != timed_out.end(); ++it)
	{
		IPollable* obj = *it;
		if (obj)
			obj->timeout();
	}
}
