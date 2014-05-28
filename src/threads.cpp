#include "sinkhole.hpp"
#include "threads.hpp"

using namespace Sinkhole;

Thread::Thread() : exit(false), started(false)
{
	ThreadEngine::threads.push_back(this);
}

Thread::~Thread()
{
	std::vector<Thread *>::iterator it = std::find(ThreadEngine::threads.begin(), ThreadEngine::threads.end(), this);
	if (it != ThreadEngine::threads.end())
		ThreadEngine::threads.erase(it);
}

void Thread::Join()
{
	this->SetExitState();
	if (this->started)
		pthread_join(this->handle, NULL);
}

void Thread::SetExitState()
{
	this->exit = true;
}

void Thread::Exit()
{
	this->SetExitState();
	pthread_exit(0);
}

bool Thread::GetExitState() const
{
	return this->exit;
}

void Thread::Signal(int sig)
{
	pthread_kill(this->handle, sig);
}

void Thread::Run()
{
}

Mutex::Mutex()
{
	pthread_mutex_init(&this->mutex, NULL);
}

Mutex::~Mutex()
{
	pthread_mutex_destroy(&this->mutex);
}

void Mutex::Lock()
{
	pthread_mutex_lock(&this->mutex);
}

void Mutex::Unlock()
{
	pthread_mutex_unlock(&this->mutex);
}

bool Mutex::TryLock()
{
	return pthread_mutex_trylock(&this->mutex) == 0;
}

Condition::Condition()
{
	pthread_cond_init(&this->cond, NULL);
}

Condition::~Condition()
{
	pthread_cond_destroy(&this->cond);
}

void Condition::Wakeup()
{
	pthread_cond_signal(&this->cond);
}

void Condition::Wait()
{
	pthread_cond_wait(&this->cond, &this->mutex);
}

std::vector<Thread *> ThreadEngine::threads;
static pthread_attr_t threadengine_attr;

static void *entry_point(void *parameter)
{
	Thread *t = static_cast<Thread *>(parameter);
	t->Run();
	t->SetExitState();
	pthread_exit(0);
}

void ThreadEngine::Init()
{
	if (pthread_attr_init(&threadengine_attr))
		throw Exception("Unable to initialize thread engine");
	if (pthread_attr_setdetachstate(&threadengine_attr, PTHREAD_CREATE_JOINABLE))
		throw Exception("Unable to set detachstate to PTHREAD_CREATE_JOINABLE");
}

void ThreadEngine::Start(Thread *t)
{
	if (pthread_create(&t->handle, &threadengine_attr, entry_point, t))
	{
		delete t;
		throw Exception("Unable to create thread: " + LastError());
	}

	t->started = true;
}

void ThreadEngine::Process()
{
	for (unsigned i = threads.size(); i > 0; --i)
	{
		Thread *t = threads[i - 1];

		if (t->GetExitState())
		{
			t->Join();
			delete t;
		}
	}
}

void ThreadEngine::Shutdown()
{
	for (unsigned i = threads.size(); i > 0; --i)
	{
		Thread *t = threads[i - 1];

		t->Join();
		delete t;
	}
}

