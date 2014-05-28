#include "sinkhole.hpp"
#include "timers.hpp"

using namespace Sinkhole;

std::vector<Timer *> TimerManager::Timers;

Timer::Timer(long time_from_now, time_t now, bool repeating)
{
	this->trigger = now + time_from_now;
	this->secs = time_from_now;
	this->repeat = repeating;
	this->settime = now;

	TimerManager::AddTimer(this);
}

Timer::~Timer()
{
	TimerManager::DelTimer(this);
}

void Timer::SetTimer(time_t t)
{
	this->trigger = t;
}

time_t Timer::GetTimer() const
{
	return this->trigger;
}

bool Timer::GetRepeat() const
{
	return this->repeat;
}

time_t Timer::GetSetTime() const
{
	return this->settime;
}

long Timer::GetSecs() const
{
	return this->secs;
}

void TimerManager::AddTimer(Timer *T)
{
	Timers.push_back(T);
	sort(Timers.begin(), Timers.end(), TimerManager::TimerComparison);
}

void TimerManager::DelTimer(Timer *T)
{
	std::vector<Timer *>::iterator i = std::find(Timers.begin(), Timers.end(), T);

	if (i != Timers.end())
		Timers.erase(i);
}

void TimerManager::Process()
{
	static time_t last_tick = 0;
	if (last_tick == curtime)
		return;
	last_tick = curtime;

	while (!Timers.empty() && curtime > Timers.front()->GetTimer())
	{
		Timer *t = Timers.front();

		t->Tick();

		if (t->GetRepeat())
		{
			t->SetTimer(curtime + t->GetSecs());
			sort(Timers.begin(), Timers.end(), TimerManager::TimerComparison);
		}
		else
			delete t;
	}
}

bool TimerManager::TimerComparison(Timer *one, Timer *two)
{
	return one->GetTimer() < two->GetTimer();
}

