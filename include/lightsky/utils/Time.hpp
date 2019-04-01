
#ifndef LS_UTILS_TIME_HPP
#define LS_UTILS_TIME_HPP

#include <chrono>
#include <ctime> // localtime, std::tm

#include "lightsky/utils/SpinLock.hpp"


namespace ls
{
namespace utils
{



/**----------------------------------------------------------------------------
 * @brief Clock Object Base Class
-----------------------------------------------------------------------------*/
template <typename precision = float, typename secondsRatio = std::ratio<1, 1>>
class Clock
{
  public:
    typedef precision precision_type;

    typedef std::chrono::steady_clock clock_type;

    typedef secondsRatio ratio_type; // seconds by default

    typedef std::chrono::duration<precision_type, ratio_type> duration_type;

    typedef std::chrono::time_point<clock_type, duration_type> time_point_type;

  private:
    static const time_point_type gProgramEpoch;

  protected:
    duration_type mTickTime;

    time_point_type mCurrPoint;

  public:
    ~Clock() noexcept;

    Clock() noexcept;

    Clock(const Clock&) noexcept;

    Clock(Clock&&) noexcept;

    Clock& operator=(const Clock&) noexcept;

    Clock& operator=(Clock&&) noexcept;

    static precision_type program_uptime() noexcept; // seconds

    static const std::tm* global_time() noexcept;

    // Please note that these are not thread safe
    static int global_second() noexcept;

    static int global_minute() noexcept;

    static int global_hour(bool daylightSavings = false, bool tfh = true) noexcept; // 24-hour format enabled by default

    static int global_day() noexcept;

    static int global_weekday() noexcept;

    static int global_month() noexcept;

    static int global_year() noexcept;

    void start() noexcept;

    void stop() noexcept;

    bool stopped() const noexcept;

    const time_point_type& current_time() const noexcept;

    void current_time(time_point_type t = duration_type(0)) noexcept;

    duration_type active_tick_time() const noexcept;

    const duration_type& tick_time() const noexcept;

    // possible use for time dilation
    void tick(precision_type timeElapsed) noexcept;

    void tick(duration_type timeElapsed) noexcept;

    void tick() noexcept;
};


/*-------------------------------------
 * Global Epoch
-------------------------------------*/
template <typename precision, typename secondsRatio>
const typename Clock<precision, secondsRatio>::time_point_type Clock<precision, secondsRatio>::gProgramEpoch = std::chrono::time_point_cast<Clock<precision, secondsRatio>::duration_type>(Clock<precision, secondsRatio>::clock_type::now());



/*-------------------------------------
 * Global Uptime
-------------------------------------*/
template <typename precision, typename secondsRatio>
inline precision Clock<precision, secondsRatio>::program_uptime() noexcept
{
    return std::chrono::duration_cast<std::chrono::duration<precision, ratio_type>>(clock_type::now() - gProgramEpoch).count();
}



/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename precision, typename secondsRatio>
Clock<precision, secondsRatio>::~Clock() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename precision, typename secondsRatio>
Clock<precision, secondsRatio>::Clock() noexcept :
    mTickTime{duration_type(0)},
    mCurrPoint{}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <typename precision, typename secondsRatio>
Clock<precision, secondsRatio>::Clock(const Clock<precision, secondsRatio>& c) noexcept :
    mTickTime{c.mTickTime},
    mCurrPoint{c.mCurrPoint}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <typename precision, typename secondsRatio>
Clock<precision, secondsRatio>::Clock(Clock<precision, secondsRatio>&& c) noexcept :
    mTickTime{c.mTickTime},
    mCurrPoint{std::move(c.mCurrPoint)}
{
    c.mTickTime = duration_type(0);
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template <typename precision, typename secondsRatio>
Clock<precision, secondsRatio>& Clock<precision, secondsRatio>::operator=(const Clock<precision, secondsRatio>& c) noexcept
{
    mTickTime = c.mTickTime;
    mCurrPoint = c.mCurrPoint;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <typename precision, typename secondsRatio>
Clock<precision, secondsRatio>& Clock<precision, secondsRatio>::operator=(Clock<precision, secondsRatio>&& c) noexcept
{
    mCurrPoint = std::move(c.mCurrPoint);

    mTickTime = c.mTickTime;
    c.mTickTime = duration_type(0);

    return *this;
}



/*-------------------------------------
 * Retrieve the global time
-------------------------------------*/
template <typename precision, typename secondsRatio>
const std::tm* Clock<precision, secondsRatio>::global_time() noexcept
{
    static SpinLock timeLock;

    timeLock.lock();
    ::time_t currTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const std::tm* ret = std::localtime(&currTime);
    timeLock.unlock();

    return ret;
}



/*-------------------------------------
 * Retrieve the global second
-------------------------------------*/
template <typename precision, typename secondsRatio>
inline int Clock<precision, secondsRatio>::global_second() noexcept
{
    return global_time()->tm_sec;
}



/*-------------------------------------
 * Retrieve the global minute
-------------------------------------*/
template <typename precision, typename secondsRatio>
inline int Clock<precision, secondsRatio>::global_minute() noexcept
{
    return global_time()->tm_min;
}



/*-------------------------------------
 * Retrieve the global hour
-------------------------------------*/
template <typename precision, typename secondsRatio>
int Clock<precision, secondsRatio>::global_hour(bool daylightSavings, bool tfh) noexcept
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
template <typename precision, typename secondsRatio>
inline int Clock<precision, secondsRatio>::global_day() noexcept
{
    return global_time()->tm_mday;
}



/*-------------------------------------
 * Retrieve the global weekday
-------------------------------------*/
template <typename precision, typename secondsRatio>
inline int Clock<precision, secondsRatio>::global_weekday() noexcept
{
    return global_time()->tm_wday;
}



/*-------------------------------------
 * Retrieve the global month
-------------------------------------*/
template <typename precision, typename secondsRatio>
inline int Clock<precision, secondsRatio>::global_month() noexcept
{
    return global_time()->tm_mon;
}



/*-------------------------------------
 * Retrieve the global year
-------------------------------------*/
template <typename precision, typename secondsRatio>
inline int Clock<precision, secondsRatio>::global_year() noexcept
{
    return global_time()->tm_year + 1900;
}



/*-------------------------------------
 * Start the clock
-------------------------------------*/
template <typename precision, typename secondsRatio>
inline void Clock<precision, secondsRatio>::start() noexcept
{
    mTickTime = duration_type(0);
    mCurrPoint = std::chrono::time_point_cast<Clock<precision, secondsRatio>::duration_type>(clock_type::now());
}



/*-------------------------------------
 * Stop
-------------------------------------*/
template <typename precision, typename secondsRatio>
inline void Clock<precision, secondsRatio>::stop() noexcept
{
    mTickTime = duration_type(0);
    mCurrPoint = time_point_type(duration_type(0));
}



/*-------------------------------------
 * Stop Check
-------------------------------------*/
template <typename precision, typename secondsRatio>
inline bool Clock<precision, secondsRatio>::stopped() const noexcept
{
    return (mTickTime == duration_type(0)) && (mCurrPoint == time_point_type(duration_type(0)));
}



/*-------------------------------------
 * Current Time
-------------------------------------*/
template <typename precision, typename secondsRatio>
inline const typename Clock<precision, secondsRatio>::time_point_type& Clock<precision, secondsRatio>::current_time() const noexcept
{
    return mCurrPoint;
}



/*-------------------------------------
 * Current Time
-------------------------------------*/
template <typename precision, typename secondsRatio>
inline void Clock<precision, secondsRatio>::current_time(time_point_type t) noexcept
{
    mCurrPoint = t;
}



/*-------------------------------------
 * Get the amount of time passed since the last tick.
-------------------------------------*/
template <typename precision, typename secondsRatio>
inline typename Clock<precision, secondsRatio>::duration_type Clock<precision, secondsRatio>::active_tick_time() const noexcept
{
    const time_point_type currTime = std::chrono::time_point_cast<Clock<precision, secondsRatio>::duration_type>(clock_type::now());
    return currTime - mCurrPoint;
}



/*-------------------------------------
 * Get the time delta from the last tick
-------------------------------------*/
template <typename precision, typename secondsRatio>
inline const typename Clock<precision, secondsRatio>::duration_type& Clock<precision, secondsRatio>::tick_time() const noexcept
{
    return mTickTime;
}



/*-------------------------------------
 * Update the current time at a fixed time delta
-------------------------------------*/
template <typename precision, typename secondsRatio>
inline void Clock<precision, secondsRatio>::tick(precision timeElapsed) noexcept
{
    mTickTime = duration_type(timeElapsed);
    mCurrPoint += mTickTime;
}



/*-------------------------------------
 * Update the current time at a fixed time delta
-------------------------------------*/
template <typename precision, typename secondsRatio>
inline void Clock<precision, secondsRatio>::tick(typename Clock<precision, secondsRatio>::duration_type timeElapsed) noexcept
{
    mTickTime = timeElapsed;
    mCurrPoint += mTickTime;
}



/*-------------------------------------
 * Update the current time
-------------------------------------*/
template <typename precision, typename secondsRatio>
inline void Clock<precision, secondsRatio>::tick() noexcept
{
    const time_point_type currTime = std::chrono::time_point_cast<Clock<precision, secondsRatio>::duration_type>(clock_type::now());
    mTickTime = currTime - mCurrPoint;
    mCurrPoint = currTime;
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_TIME_HPP */
