#include "Pollable.hpp"

Pollable::Pollable(fd fd_) : _fd(fd_) {}

Pollable::~Pollable() {}

Pollable::operator int() const 
{
	return (_fd);
}

Pollable::operator fd() const
{
	return (_fd);
}
