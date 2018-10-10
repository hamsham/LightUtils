
#ifndef LS_UTILS_TIME_HPP
#define LS_UTILS_TIME_HPP

#include <vector>
#include <chrono>
#include <ctime>



namespace ls
{
namespace utils
{



/**----------------------------------------------------------------------------
 * @brief Clock Object Base Class
-----------------------------------------------------------------------------*/
class Clock
{
  public:

    enum : unsigned
    {
        CLOCK_STOP_AT_TIME_POINT = 0x01,
        CLOCK_PAUSED = 0x02
    };

    typedef double precision_type;

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

    template<typename precision_t = precision_type, typename ratio_t = ratio_type>
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

    void current_time(precision_type t = 0.0);

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



// default template args:
// precision_t: hr_precision
// ratio_t:     ratio_type
template<typename precision_t, typename ratio_t>
inline Clock::precision_type Clock::program_uptime()
{
    return std::chrono::duration_cast<std::chrono::duration<precision_t, ratio_t>>(clock_type::now() - gProgramEpoch).count();
}



/**----------------------------------------------------------------------------
 * @brief Timer Class
 *
 * This object can be used to count down from a specific time point,
-----------------------------------------------------------------------------*/
class Timer : virtual public Clock
{
  private:
    Clock::precision_type mStartTime;

  public:
    virtual ~Timer() override;

    Timer();

    Timer(const Timer&);

    Timer(Timer&&);

    Timer& operator=(const Timer&);

    Timer& operator=(Timer&&);

    void start() override;

    void start_time(precision_type t);

    precision_type start_time() const;

    bool finished() const;

    void update() override;

    void tick(precision_type timeElapsed) override;
};



/**----------------------------------------------------------------------------
 * @brief Stopwatch Class
 *
 * This object can be used to count a time upwards, keeping track of lap times
 * through a vector.
-----------------------------------------------------------------------------*/
class StopWatch : virtual public Clock
{
  private:
    std::vector<Clock::precision_type> mLaps;

  public:
    virtual ~StopWatch() override;

    StopWatch();

    StopWatch(const StopWatch&);

    StopWatch(StopWatch&&);

    StopWatch& operator=(const StopWatch&);

    StopWatch& operator=(StopWatch&&);

    precision_type lap_time(unsigned lapIndex) const;

    unsigned num_laps() const;

    void clear();

    void lap();
};



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_TIME_HPP */
