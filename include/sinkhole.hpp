#ifndef SINKHOLE_H
#define SINKHOLE_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <stack>
#include <algorithm>

#include <signal.h>

namespace Sinkhole
{
	extern bool shutting_down;
	extern bool nofork, debug;
	extern std::string LastError();
	extern time_t curtime;

	class Exception
	{
		std::string reason;
	 public:
		Exception(const std::string &r);
		virtual ~Exception() throw();

		const std::string &GetReason() const;
	};

	class Signal
	{
		static std::vector<Signal *> SignalHandlers;
		static void SignalHandler(int);

		struct sigaction action;
		sig_atomic_t called;
	 public:
	 	static void Process();

	 	int signal;

		Signal(int s);
		~Signal();
		virtual void OnSignal() = 0;
	};
}

#endif // SINKHOLE_H
