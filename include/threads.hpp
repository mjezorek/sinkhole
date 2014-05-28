#ifndef THREADS_H
#define THREADS_H

#include <pthread.h>

namespace Sinkhole
{
	typedef pthread_t ThreadHandle;
	typedef pthread_mutex_t MutexHandle;
	typedef pthread_cond_t CondHandle;

	class Thread
	{
	 private:
	 	friend class ThreadEngine;

		/* Set to true to tell the thread to finish and we are waiting for it */
		bool exit;

		/* Set to true when the thread is started */
		bool started;

		/* Handle for this thread */
		ThreadHandle handle;

	 public:
		/** Threads constructor
		 */
		Thread();

		/** Threads destructor
		 */
		virtual ~Thread();

		/** Join to the thread, sets the exit state to true
		 */
		void Join();

		/** Sets the exit state as true informing the thread we want it to shut down
		 */
		void SetExitState();

		/** Exit the thread. Note that the thread still must be joined to free resources!
		 */
		void Exit();

		/** Returns the exit state of the thread
		 * @return true if we want to exit
		 */
		bool GetExitState() const;

		/** Send a signal to this thread
		 * @param sig The signal
		 */
		void Signal(int sig);

		/** Called to run the thread, should be overloaded
		 */
		virtual void Run();
	};

	class Mutex
	{
	 protected:
		/* A mutex, used to keep threads in sync */
		MutexHandle mutex;

	 public:
		/** Constructor
		 */
		Mutex();

		/** Destructor
		 */
		~Mutex();

		/** Attempt to lock the mutex, will hang until a lock can be achieved
		 */
		void Lock();

		/** Unlock the mutex, it must be locked first
		 */
		void Unlock();

		/** Attempt to lock the mutex, will return true on success and false on fail
		 * Does not block
		 * @return true or false
		 */
		bool TryLock();
	};

	class Condition : public Mutex
	{
	 private:
		/* A condition */
		CondHandle cond;

	 public:
		/** Constructor
		 */
		Condition();

		/** Destructor
		 */
		~Condition();

		/** Called to wakeup the waiter
		 */
		void Wakeup();

		/** Called to wait for a Wakeup() call
		 */
		void Wait();
	};

	class ThreadEngine
	{
		friend class Thread;

		/* Vector of threads */
		static std::vector<Thread *> threads;
	 public:

		/** Initialize the thread engine
		 */
		static void Init();

		/** Start a new thread
		 * @param thread A pointer to a newley allocated thread
		 */
		static void Start(Thread *thread);

		/** Check for finished threads
		 */
		static void Process();

		/** Shutdown all running threads
		 */
		static void Shutdown();
	};
}

#endif // THREADS_H

