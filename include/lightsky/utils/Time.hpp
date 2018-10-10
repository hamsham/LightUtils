
#ifndef LS_UTILS_TIME_HPP
#define LS_UTILS_TIME_HPP

#include <vector>
#include <chrono>
#include <ctime> // localtime, std::tm

#include "lightsky/utils/WorkerThread.hpp" // SpinLock


namespace ls
{
namespace utils
{



/**----------------------------------------------------------------------------
 * @brief Clock Object Base Class
-----------------------------------------------------------------------------*/
template <typename precision = float>
class Clock
{
  public:

    enum : unsigned
    {
        CLOCK_STOP_AT_TIME_POINT = 0x01,
        CLOCK_PAUSED = 0x02
    };

    typedef precision precision_type;

    typedef std::chrono::high_resolution_clock clock_type;

    typedef std::ratio<1, 1> ratio_type; // seconds

    typedef std::chrono::duration<precision_type, ratio_type> duration_type;

    typedef std::chrono::time_point<clock_type, duration_type> time_point_type;

  private:
    static const time_point_type gProgramEpoch;

    static const std::tm* global_time();

  protected:
    unsigned mFlags;

    time_point_type mCurrPoint;

    precision_type mCurrTime;

    precision_type mTickTime;

    precision_type mStopTime;

  public:
    virtual ~Clock();

    Clock();

    Clock(const Clock&);

    Clock(Clock&&);

    Clock& operator=(const Clock&);

    Clock& operator=(Clock&&);

    static precision_type program_uptime(); // seconds

    // Please note that these are not thread safe
    static int global_second();

    static int global_minute();

    static int global_hour(bool daylightSavings = false, bool tfh = true); // 24-hour format enabled by default

    static int global_day();

    static int global_weekday();

    static int global_month();

    static int global_year();

    virtual void start();

    virtual void pause();

    virtual void stop();

    precision_type current_time() const;

    void current_time(precision_type t = precision{0});

    precision_type active_tick_time() const;

    precision_type tick_time() const;

    void stop_time(precision_type t);

    precision_type stop_time() const;

    void flags(unsigned f);

    unsigned flags() const;

    bool paused() const;

    bool will_stop() const;

    // possible use for time dilation
    virtual void update();

    virtual void tick(precision_type timeElapsed);
};


/*-------------------------------------
 * Global Epoch
-------------------------------------*/
template <typename precision>
const typename Clock<precision>::time_point_type Clock<precision>::gProgramEpoch = std::chrono::time_point_cast<Clock<precision>::duration_type>(Clock<precision>::clock_type::now());



/*-------------------------------------
 * Global Uptime
-------------------------------------*/
template <typename precision>
inline precision Clock<precision>::program_uptime()
{
    return std::chrono::duration_cast<std::chrono::duration<precision, ratio_type>>(clock_type::now() - gProgramEpoch).count();
}



/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename precision>
Clock<precision>::~Clock()
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename precision>
Clock<precision>::Clock() :
    mFlags{CLOCK_PAUSED},
    mCurrPoint{},
    mCurrTime{precision{0}},
    mTickTime{precision{0}},
    mStopTime{precision{0}}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <typename precision>
Clock<precision>::Clock(const Clock<precision>& c) :
    mFlags{c.mFlags},
    mCurrPoint{c.mCurrPoint},
    mCurrTime{c.mCurrTime},
    mTickTime{c.mTickTime},
    mStopTime{c.mStopTime}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <typename precision>
Clock<precision>::Clock(Clock<precision>&& c) :
    mFlags{c.mFlags},
    mCurrPoint{std::move(c.mCurrPoint)},
    mCurrTime{c.mCurrTime},
    mTickTime{c.mTickTime},
    mStopTime{c.mStopTime}
{
    c.mFlags = CLOCK_PAUSED;
    c.mCurrTime = precision{0};
    c.mTickTime = precision{0};
    c.mStopTime = precision{0};
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template <typename precision>
Clock<precision>& Clock<precision>::operator=(const Clock<precision>& c)
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
template <typename precision>
Clock<precision>& Clock<precision>::operator=(Clock<precision>&& c)
{
    mFlags = c.mFlags;
    c.mFlags = CLOCK_PAUSED;

    mCurrPoint = std::move(c.mCurrPoint);

    mCurrTime = c.mCurrTime;
    c.mCurrTime = precision{0};

    mTickTime = c.mTickTime;
    c.mTickTime = precision{0};

    mStopTime = c.mStopTime;
    c.mStopTime = precision{0};

    return *this;
}



/*-------------------------------------
 * Retrieve the global time
-------------------------------------*/
template <typename precision>
const std::tm* Clock<precision>::global_time()
{
    static SpinLock timeLock;

    timeLock.lock();
    ::time_t currTime = std::chrono::system_clock::to_time_t(Clock<precision>::clock_type::now());
    const std::tm* ret = std::localtime(&currTime);
    timeLock.unlock();

    return ret;
}



/*-------------------------------------
 * Retrieve the global second
-------------------------------------*/
template <typename precision>
inline int Clock<precision>::global_second()
{
    return global_time()->tm_sec;
}



/*-------------------------------------
 * Retrieve the global minute
-------------------------------------*/
template <typename precision>
inline int Clock<precision>::global_minute()
{
    return global_time()->tm_min;
}



/*-------------------------------------
 * Retrieve the global hour
-------------------------------------*/
template <typename precision>
int Clock<precision>::global_hour(bool daylightSavings, bool tfh)
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
template <typename precision>
inline int Clock<precision>::global_day()
{
    return global_time()->tm_mday;
}



/*-------------------------------------
 * Retrieve the global weekday
-------------------------------------*/
template <typename precision>
inline int Clock<precision>::global_weekday()
{
    return global_time()->tm_wday;
}



/*-------------------------------------
 * Retrieve the global month
-------------------------------------*/
template <typename precision>
inline int Clock<precision>::global_month()
{
    return global_time()->tm_mon;
}



/*-------------------------------------
 * Retrieve the global year
-------------------------------------*/
template <typename precision>
inline int Clock<precision>::global_year()
{
    return global_time()->tm_year + 1900;
}



/*-------------------------------------
 * Start the clock
-------------------------------------*/
template <typename precision>
inline void Clock<precision>::start()
{
    if (mFlags & CLOCK_PAUSED)
    {
        mFlags ^= CLOCK_PAUSED;
    }

    mTickTime = mCurrTime = precision{0};
    mCurrPoint = std::chrono::time_point_cast<Clock<precision>::duration_type>(clock_type::now());
}



/*-------------------------------------
 * Pause
-------------------------------------*/
template <typename precision>
inline void Clock<precision>::pause()
{
    mFlags |= CLOCK_PAUSED;
    mTickTime = precision{0};
    mCurrPoint = std::chrono::time_point_cast<Clock<precision>::duration_type>(clock_type::now());
}



/*-------------------------------------
 * Stop
-------------------------------------*/
template <typename precision>
inline void Clock<precision>::stop()
{
    mFlags |= CLOCK_PAUSED;
    mTickTime = mCurrTime = precision{0};
    //mCurrPoint = c_timeObject::duration_type( precision{0} );
}



/*-------------------------------------
 * Get the current time
-------------------------------------*/
template <typename precision>
inline precision Clock<precision>::current_time() const
{
    return mCurrTime;
}

/*-------------------------------------
 * Set the current time manually
-------------------------------------*/
template <typename precision>
inline void Clock<precision>::current_time(precision t)
{
    mCurrTime = t;
}



/*-------------------------------------
 * Get the amount of time passed since the last update.
-------------------------------------*/
template <typename precision>
inline precision Clock<precision>::active_tick_time() const
{
    return mFlags ? precision{0} : std::chrono::duration_cast<duration_type>(clock_type::now() - mCurrPoint).count();
}



/*-------------------------------------
 * Get the time delta from the last update
-------------------------------------*/
template <typename precision>
inline precision Clock<precision>::tick_time() const
{
    return mTickTime;
}



/*-------------------------------------
 * Set the stop time
-------------------------------------*/
template <typename precision>
inline void Clock<precision>::stop_time(precision t)
{
    mStopTime = t;
}



/*-------------------------------------
 * Get the current stop time
-------------------------------------*/
template <typename precision>
inline precision Clock<precision>::stop_time() const
{
    return mStopTime;
}



/*-------------------------------------
 * Set the current time flags
-------------------------------------*/
template <typename precision>
inline void Clock<precision>::flags(unsigned flags)
{
    mFlags = flags;
}



/*-------------------------------------
 * Get the current time flags
-------------------------------------*/
template <typename precision>
inline unsigned Clock<precision>::flags() const
{
    return mFlags;
}



/*-------------------------------------
 * Check if the clock is paused
-------------------------------------*/
template <typename precision>
inline bool Clock<precision>::paused() const
{
    return (mFlags & CLOCK_PAUSED) != 0;
}



/*-------------------------------------
 * Determine if the clock is going to stop
-------------------------------------*/
template <typename precision>
inline bool Clock<precision>::will_stop() const
{
    return (mFlags & CLOCK_STOP_AT_TIME_POINT) != 0;
}



/*-------------------------------------
 * Update the current time
-------------------------------------*/
template <typename precision>
void Clock<precision>::update()
{
    if (paused())
    {
        return;
    }

    mTickTime = std::chrono::duration_cast<duration_type>(clock_type::now() - mCurrPoint).count();
    mCurrTime += mTickTime;
    mCurrPoint = std::chrono::time_point_cast<Clock<precision>::duration_type>(clock_type::now());

    if ((mFlags & CLOCK_STOP_AT_TIME_POINT) && (mCurrTime >= mStopTime))
    {
        mFlags |= CLOCK_PAUSED;
    }
}



/*-------------------------------------
 * Update the current time at a fixed time delta
-------------------------------------*/
template <typename precision>
void Clock<precision>::tick(precision timeElapsed)
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



/**----------------------------------------------------------------------------
 * @brief Timer Class
 *
 * This object can be used to count down from a specific time point,
-----------------------------------------------------------------------------*/
template <typename precision>
class Timer : virtual public Clock<precision>
{
  private:
    typename Clock<precision>::precision_type mStartTime;

  public:
    virtual ~Timer() override;

    Timer();

    Timer(const Timer&);

    Timer(Timer&&);

    Timer& operator=(const Timer&);

    Timer& operator=(Timer&&);

    virtual void start() override;

    void start_time(precision t);

    precision start_time() const;

    bool finished() const;

    virtual void update() override;

    virtual void tick(precision timeElapsed) override;
};



/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename precision>
Timer<precision>::~Timer()
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename precision>
Timer<precision>::Timer() :
    Clock<precision>{},
    mStartTime{precision{0}}
{
    this->mFlags |= Clock<precision>::CLOCK_STOP_AT_TIME_POINT;
}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <typename precision>
Timer<precision>::Timer(const Timer<precision>& t) :
    Clock<precision>{t},
    mStartTime(t.mStartTime)
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <typename precision>
Timer<precision>::Timer(Timer<precision>&& t) :
    Clock<precision>{std::move(t)},
    mStartTime(t.mStartTime)
{
    t.mStartTime = precision{0};
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template <typename precision>
Timer<precision>& Timer<precision>::operator=(const Timer<precision>& t)
{
    Clock<precision>::operator=(t);
    mStartTime = t.mStartTime;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <typename precision>
Timer<precision>& Timer<precision>::operator=(Timer<precision>&& t)
{
    Clock<precision>::operator=(std::move(t));

    mStartTime = t.mStartTime;
    t.mStartTime = precision{0};

    return *this;
}



/*-------------------------------------
 * Start the timer
-------------------------------------*/
template <typename precision>
inline void Timer<precision>::start()
{
    if (this->mFlags & Clock<precision>::CLOCK_PAUSED)
    {
        this->mFlags ^= Clock<precision>::CLOCK_PAUSED;
    }

    this->mTickTime = precision{0};
    this->mCurrTime = mStartTime;
    this->mCurrPoint = std::chrono::time_point_cast<Clock<precision>::duration_type>(Clock<precision>::clock_type::now());
}



/*-------------------------------------
 * Set the start time
-------------------------------------*/
template <typename precision>
inline void Timer<precision>::start_time(precision t)
{
    mStartTime = t;
}



/*-------------------------------------
 * Get the start time
-------------------------------------*/
template <typename precision>
inline precision Timer<precision>::start_time() const
{
    return mStartTime;
}



/*-------------------------------------
 * Check if the timer has finished
-------------------------------------*/
template <typename precision>
inline bool Timer<precision>::finished() const
{
    if ((this->mFlags & Clock<precision>::CLOCK_STOP_AT_TIME_POINT) != 0)
    {
        return this->mCurrTime <= this->mStopTime;
    }
    return false;
}



/*-------------------------------------
 * Update the timer
-------------------------------------*/
template <typename precision>
void Timer<precision>::update()
{
    if (this->paused())
    {
        return;
    }

    if (this->mStopTime >= mStartTime)
    {
        return;
    }

    this->mTickTime = std::chrono::duration_cast<Timer::duration_type>(Timer::clock_type::now() - this->mCurrPoint).count();
    this->mCurrTime -= this->mTickTime;
    this->mCurrPoint = std::chrono::time_point_cast<Clock<precision>::duration_type>(Clock<precision>::clock_type::now());

    if (this->finished())
    {
        this->mFlags |= Clock<precision>::CLOCK_PAUSED;
    }
}



/*-------------------------------------
 * Update the timer at a fixed delta
-------------------------------------*/
template <typename precision>
void Timer<precision>::tick(precision timeElapsed)
{
    if (this->paused())
    {
        return;
    }

    if (this->mStopTime > mStartTime)
    {
        return;
    }

    this->mTickTime = timeElapsed;
    this->mCurrTime -= this->mTickTime;
    this->mCurrPoint -= std::chrono::time_point_cast<Clock<precision>::duration_type>(Clock<precision>::clock_type::now());

    if (this->finished())
    {
        this->mFlags |= Clock<precision>::CLOCK_PAUSED;
    }
}



/**----------------------------------------------------------------------------
 * @brief Stopwatch Class
 *
 * This object can be used to count a time upwards, keeping track of lap times
 * through a vector.
-----------------------------------------------------------------------------*/
template <typename precision>
class StopWatch : virtual public Clock<precision>
{
  private:
    std::vector<precision> mLaps;

  public:
    virtual ~StopWatch() override;

    StopWatch();

    StopWatch(const StopWatch&);

    StopWatch(StopWatch&&);

    StopWatch& operator=(const StopWatch&);

    StopWatch& operator=(StopWatch&&);

    precision lap_time(unsigned lapIndex) const;

    unsigned num_laps() const;

    void clear();

    void lap();
};



/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename precision>
StopWatch<precision>::~StopWatch()
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename precision>
StopWatch<precision>::StopWatch() :
    Clock<precision>{},
    mLaps{}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <typename precision>
StopWatch<precision>::StopWatch(const StopWatch<precision>& sw) :
    Clock<precision>{sw},
    mLaps{sw.mLaps}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <typename precision>
StopWatch<precision>::StopWatch(StopWatch<precision>&& sw) :
    Clock<precision>{std::move(sw)},
    mLaps{std::move(sw.mLaps)}
{}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template <typename precision>
StopWatch<precision>& StopWatch<precision>::operator=(const StopWatch<precision>& sw)
{
    Clock<precision>::operator=(sw);
    mLaps = sw.mLaps;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <typename precision>
StopWatch<precision>& StopWatch<precision>::operator=(StopWatch<precision>&& sw)
{
    Clock<precision>::operator=(std::move(sw));
    mLaps = std::move(sw.mLaps);

    return *this;
}



/*-------------------------------------
 * Get the time of a single lap
-------------------------------------*/
template <typename precision>
inline precision StopWatch<precision>::lap_time(unsigned lapIndex) const
{
    if (lapIndex > mLaps.size())
    {
        return precision{0};
    }
    return mLaps[lapIndex];
}



/*-------------------------------------
 * Get the number of laps
-------------------------------------*/
template <typename precision>
inline unsigned StopWatch<precision>::num_laps() const
{
    return mLaps.size();
}



/*-------------------------------------
 * Clear all laps
-------------------------------------*/
template <typename precision>
inline void StopWatch<precision>::clear()
{
    mLaps.clear();
}



/*-------------------------------------
 * Store a lap
-------------------------------------*/
template <typename precision>
inline void StopWatch<precision>::lap()
{
    mLaps.push_back(this->mCurrTime);
    this->mCurrTime = precision{0};
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_TIME_HPP */
