#include "sinkhole.hpp"
#include <string.h>
#include <errno.h>

bool Sinkhole::shutting_down = false;
bool Sinkhole::nofork = 0, Sinkhole::debug = 0;
time_t Sinkhole::curtime = time(NULL);

using namespace Sinkhole;

std::string Sinkhole::LastError()
{
	return strerror(errno);
}

Sinkhole::Exception::Exception(const std::string &r) : reason(r)
{
}

Sinkhole::Exception::~Exception() throw()
{
}

const std::string &Sinkhole::Exception::GetReason() const
{
	return this->reason;
}

std::vector<Signal *> Sinkhole::Signal::SignalHandlers;

void Sinkhole::Signal::SignalHandler(int signum)
{
	for (unsigned i = 0, j = SignalHandlers.size(); i < j; ++i)
		if (SignalHandlers[i]->signal == signum)
			SignalHandlers[i]->called = true;
}

void Sinkhole::Signal::Process()
{
	for (unsigned i = 0, j = SignalHandlers.size(); i < j; ++i)
		if (SignalHandlers[i]->called == true)
		{
			Signal *s = SignalHandlers[i];
			s->called = false;
			s->OnSignal();
		}
}

Sinkhole::Signal::Signal(int s) : called(false), signal(s)
{
	this->action.sa_flags = 0;
	sigemptyset(&this->action.sa_mask);
	this->action.sa_handler = SignalHandler;

	sigaction(s, &this->action, NULL);

	SignalHandlers.push_back(this);
}

Sinkhole::Signal::~Signal()
{
	std::vector<Signal *>::iterator it = std::find(SignalHandlers.begin(), SignalHandlers.end(), this);
	if (it != SignalHandlers.end())
		SignalHandlers.erase(it);
}

