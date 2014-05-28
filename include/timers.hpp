#ifndef TIMERS_H
#define TIMERS_H

namespace Sinkhole
{
	class Timer
	{
	 private:
		/** The time this was created
		 */
		time_t settime;

		/** The triggering time
		 */
		time_t trigger;

		/** Numer of seconds between triggers
		 */
		long secs;

		/** True if this is a repeating timer
		 */
		bool repeat;

	 public:
		/** Default constructor, initializes the triggering time
		 * @param time_from_now The number of seconds from now to trigger the timer
		 * @param now The time now
		 * @param repeating Repeat this timer every time_from_now if this is true
		 */
		Timer(long time_from_now, time_t now = Sinkhole::curtime, bool repeating = false);

		/** Default destructor, removes the timer from the list
		 */
		virtual ~Timer();

		/** Set the trigger time to a new value
		 * @param t The new time
		 */
		void SetTimer(time_t t);

		/** Retrieve the triggering time
		 * @return The trigger time
		 */
		time_t GetTimer() const;

		/** Returns true if the timer is set to repeat
		 * @return Returns true if the timer is set to repeat
		 */
		bool GetRepeat() const;

		/** Returns the interval between ticks
		 * @return The interval
		 */
		long GetSecs() const;

		/** Returns the time this timer was created
		 * @return The time this timer was created
		 */
		time_t GetSetTime() const;

		/** Called when the timer ticks
		 */
		virtual void Tick() = 0;
	};

	/** This class manages sets of Timers, and triggers them at their defined times.
	 * This will ensure timers are not missed, as well as removing timers that have
	 * expired and allowing the addition of new ones.
	 */
	class TimerManager
	{
	 private:
		/** A list of timers
		 */
		static std::vector<Timer *> Timers;
	 public:
		/** Add a timer to the list
		 * @param T A Timer derived class to add
		 */
		static void AddTimer(Timer *T);

		/** Deletes a timer
		 * @param T A Timer derived class to delete
		 */
		static void DelTimer(Timer *T);

		/** Tick all pending timers
		 */
		static void Process();

		/** Compares two timers
		 */
		static bool TimerComparison(Timer *one, Timer *two);
	};
}

#endif // TIMERS_H
