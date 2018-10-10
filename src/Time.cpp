
#include <utility> // std::move

#include "lightsky/utils/Time.hpp"
#include "lightsky/utils/WorkerThread.hpp" // SpinLock

using namespace std::chrono;

namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 *		Clock Object Base Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Global Epoch
-------------------------------------*/
const Clock::time_point_type Clock::gProgramEpoch = Clock::clock_type::now();



/*-------------------------------------
 * Destructor
-------------------------------------*/
Clock::~Clock()
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
Clock::Clock() :
    mFlags{CLOCK_PAUSED},
    mCurrPoint{},
    mCurrTime{0.0},
    mTickTime{0.0},
    mStopTime{0.0}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
Clock::Clock(const Clock& c) :
    mFlags{c.mFlags},
    mCurrPoint{c.mCurrPoint},
    mCurrTime{c.mCurrTime},
    mTickTime{c.mTickTime},
    mStopTime{c.mStopTime}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
Clock::Clock(Clock&& c) :
    mFlags{c.mFlags},
    mCurrPoint{std::move(c.mCurrPoint)},
    mCurrTime{c.mCurrTime},
    mTickTime{c.mTickTime},
    mStopTime{c.mStopTime}
{
    c.mFlags = CLOCK_PAUSED;
    c.mCurrTime = 0.0;
    c.mTickTime = 0.0;
    c.mStopTime = 0.0;
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
Clock& Clock::operator=(const Clock& c)
{
    mFlags = c.mFlags;
    mCurrPoint = c.mCurrPoint;
    mCurrTime = c.mCurrTime;
    mTickTime = c.mTickTime;
    mStopTime = c.mStopTime;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
Clock& Clock::operator=(Clock&& c)
{
    mFlags = c.mFlags;
    c.mFlags = CLOCK_PAUSED;

    mCurrPoint = std::move(c.mCurrPoint);

    mCurrTime = c.mCurrTime;
    c.mCurrTime = 0.0;

    mTickTime = c.mTickTime;
    c.mTickTime = 0.0;

    mStopTime = c.mStopTime;
    c.mStopTime = 0.0;

    return *this;
}



/*-------------------------------------
 * Retrieve the global time
-------------------------------------*/
const std::tm* Clock::global_time()
{
    static SpinLock timeLock;

    timeLock.lock();
    ::time_t currTime = system_clock::to_time_t(system_clock::now());
    const std::tm* ret = localtime(&currTime);
    timeLock.unlock();

    return ret;
}



/*-------------------------------------
 * Retrieve the global second
-------------------------------------*/
int Clock::global_second()
{
    return global_time()->tm_sec;
}



/*-------------------------------------
 * Retrieve the global minute
-------------------------------------*/
int Clock::global_minute()
{
    return global_time()->tm_min;
}



/*-------------------------------------
 * Retrieve the global hour
-------------------------------------*/
int Clock::global_hour(bool daylightSavings, bool tfh)
{
    const std::tm* timeInfo = global_time();
    int hour = timeInfo->tm_hour;

    // subtract 12 hours if twenty-four hour mode is disabled
    hour -= -(!tfh && (hour > 12)) & 12;

    return hour + (daylightSavings ? 1 : 0);
}



/*-------------------------------------
 * Retrieve the global day
-------------------------------------*/
int Clock::global_day()
{
    return global_time()->tm_mday;
}



/*-------------------------------------
 * Retrieve the global weekday
-------------------------------------*/
int Clock::global_weekday()
{
    return global_time()->tm_wday;
}



/*-------------------------------------
 * Retrieve the global month
-------------------------------------*/
int Clock::global_month()
{
    return global_time()->tm_mon;
}



/*-------------------------------------
 * Retrieve the global year
-------------------------------------*/
int Clock::global_year()
{
    return global_time()->tm_year + 1900;
}



/*-------------------------------------
 * Start the clock
-------------------------------------*/
void Clock::start()
{
    if (mFlags & CLOCK_PAUSED)
    {
        mFlags ^= CLOCK_PAUSED;
    }

    mTickTime = mCurrTime = 0.0;
    mCurrPoint = Clock::clock_type::now();
}



/*-------------------------------------
 * Pause
-------------------------------------*/
void Clock::pause()
{
    mFlags |= CLOCK_PAUSED;
    mTickTime = 0.0;
    mCurrPoint = Clock::clock_type::now();
}



/*-------------------------------------
 * Stop
-------------------------------------*/
void Clock::stop()
{
    mFlags |= CLOCK_PAUSED;
    mTickTime = mCurrTime = 0.0;
    //mCurrPoint = c_timeObject::duration_type( 0.0 );
}



/*-------------------------------------
 * Get the current time
-------------------------------------*/
Clock::precision_type Clock::current_time() const
{
    return mCurrTime;
}

/*-------------------------------------
 * Set the current time manually
-------------------------------------*/
void Clock::current_time(Clock::precision_type t)
{
    mCurrTime = t;
}



/*-------------------------------------
 * Get the amount of time passed since the last update.
-------------------------------------*/
Clock::precision_type Clock::active_tick_time() const
{
    return mFlags ? 0.0 : duration_cast<duration_type>(clock_type::now() - mCurrPoint).count();
}



/*-------------------------------------
 * Get the time delta from the last update
-------------------------------------*/
Clock::precision_type Clock::tick_time() const
{
    return mTickTime;
}



/*-------------------------------------
 * Set the stop time
-------------------------------------*/
void Clock::stop_time(precision_type t)
{
    mStopTime = t;
}



/*-------------------------------------
 * Get the current stop time
-------------------------------------*/
Clock::precision_type Clock::stop_time() const
{
    return mStopTime;
}



/*-------------------------------------
 * Set the current time flags
-------------------------------------*/
void Clock::flags(unsigned flags)
{
    mFlags = flags;
}



/*-------------------------------------
 * Get the current time flags
-------------------------------------*/
unsigned Clock::flags() const
{
    return mFlags;
}



/*-------------------------------------
 * Check if the clock is paused
-------------------------------------*/
bool Clock::paused() const
{
    return (mFlags & CLOCK_PAUSED) != 0;
}



/*-------------------------------------
 * Determine if the clock is going to stop
-------------------------------------*/
bool Clock::will_stop() const
{
    return (mFlags & CLOCK_STOP_AT_TIME_POINT) != 0;
}



/*-------------------------------------
 * Update the current time
-------------------------------------*/
void Clock::update()
{
    if (paused())
    {
        return;
    }

    mTickTime = duration_cast<duration_type>(clock_type::now() - mCurrPoint).count();

    mCurrTime += mTickTime;
    mCurrPoint = clock_type::now();

    if ((mFlags & CLOCK_STOP_AT_TIME_POINT) && (mCurrTime >= mStopTime))
    {
        mFlags |= CLOCK_PAUSED;
    }
}



/*-------------------------------------
 * Update the current time at a fixed time delta
-------------------------------------*/
void Clock::tick(Clock::precision_type timeElapsed)
{
    if (paused())
    {
        return;
    }

    mTickTime = timeElapsed;
    mCurrTime += mTickTime;
    mCurrPoint += duration_type(timeElapsed);

    if ((mFlags & CLOCK_STOP_AT_TIME_POINT) && (mCurrTime >= mStopTime))
    {
        mFlags |= CLOCK_PAUSED;
    }
}



/*-----------------------------------------------------------------------------
 * TIMER CLASS
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
Timer::~Timer()
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
Timer::Timer() :
    Clock{},
    mStartTime{0.0}
{
    mFlags |= CLOCK_STOP_AT_TIME_POINT;
}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
Timer::Timer(const Timer& t) :
    Clock{t},
    mStartTime(t.mStartTime)
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
Timer::Timer(Timer&& t) :
    Clock{std::move(t)},
    mStartTime(t.mStartTime)
{
    t.mStartTime = 0.0;
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
Timer& Timer::operator=(const Timer& t)
{
    Clock::operator=(t);
    mStartTime = t.mStartTime;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
Timer& Timer::operator=(Timer&& t)
{
    Clock::operator=(std::move(t));

    mStartTime = t.mStartTime;
    t.mStartTime = 0.0;

    return *this;
}



/*-------------------------------------
 * Start the timer
-------------------------------------*/
void Timer::start()
{
    if (mFlags & CLOCK_PAUSED)
    {
        mFlags ^= CLOCK_PAUSED;
    }

    mTickTime = 0.0;
    mCurrTime = mStartTime;
    mCurrPoint = Clock::clock_type::now();
}



/*-------------------------------------
 * Set the start time
-------------------------------------*/
void Timer::start_time(Clock::precision_type t)
{
    mStartTime = t;
}



/*-------------------------------------
 * Get the start time
-------------------------------------*/
Clock::precision_type Timer::start_time() const
{
    return mStartTime;
}



/*-------------------------------------
 * Check if the timer has finished
-------------------------------------*/
bool Timer::finished() const
{
    if ((mFlags & CLOCK_STOP_AT_TIME_POINT) != 0)
    {
        return mCurrTime <= mStopTime;
    }
    return false;
}



/*-------------------------------------
 * Update the timer
-------------------------------------*/
void Timer::update()
{
    if (paused())
    {
        return;
    }

    if (mStopTime >= mStartTime)
    {
        return;
    }

    mTickTime = duration_cast<Clock::duration_type>(Clock::clock_type::now() - mCurrPoint).count();
    mCurrTime -= mTickTime;
    mCurrPoint = Clock::clock_type::now();

    if (finished())
    {
        mFlags |= CLOCK_PAUSED;
    }
}



/*-------------------------------------
 * Update the timer at a fixed delta
-------------------------------------*/
void Timer::tick(Clock::precision_type timeElapsed)
{
    if (paused())
    {
        return;
    }

    if (mStopTime > mStartTime)
    {
        return;
    }

    mTickTime = timeElapsed;
    mCurrTime -= mTickTime;
    mCurrPoint -= Clock::duration_type(timeElapsed);

    if (finished())
    {
        mFlags |= CLOCK_PAUSED;
    }
}



/*-----------------------------------------------------------------------------
 * STOPWATCH CLASS
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
StopWatch::~StopWatch()
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
StopWatch::StopWatch() :
    Clock(),
    mLaps()
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
StopWatch::StopWatch(const StopWatch& sw) :
    Clock{sw},
    mLaps{sw.mLaps}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
StopWatch::StopWatch(StopWatch&& sw) :
    Clock{std::move(sw)},
    mLaps{std::move(sw.mLaps)}
{}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
StopWatch& StopWatch::operator=(const StopWatch& sw)
{
    Clock::operator=(sw);
    mLaps = sw.mLaps;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
StopWatch& StopWatch::operator=(StopWatch&& sw)
{
    Clock::operator=(std::move(sw));
    mLaps = std::move(sw.mLaps);

    return *this;
}



/*-------------------------------------
 * Get the time of a single lap
-------------------------------------*/
Clock::precision_type StopWatch::lap_time(unsigned lapIndex) const
{
    if (lapIndex > mLaps.size())
    {
        return 0.0;
    }
    return mLaps[lapIndex];
}



/*-------------------------------------
 * Get the number of laps
-------------------------------------*/
unsigned StopWatch::num_laps() const
{
    return mLaps.size();
}



/*-------------------------------------
 * Clear all laps
-------------------------------------*/
void StopWatch::clear()
{
    mLaps.clear();
}



/*-------------------------------------
 * Store a lap
-------------------------------------*/
void StopWatch::lap()
{
    mLaps.push_back(mCurrTime);
    mCurrTime = 0.0;
}



} // end utils namespace
} // end ls namespace
