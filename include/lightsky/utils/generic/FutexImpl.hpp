
#include <climits> // INT32_MAX
#include <thread> // std::this_thread::yield()



namespace ls
{
namespace utils
{



/*-------------------------------------
 * Futex Lock
-------------------------------------*/
inline void Futex::lock() noexcept
{
    int32_t val = 0;

    for (unsigned i = 0; i < INT8_MAX; ++i)
    {
        if (!mLock.compare_exchange_weak(val, INT32_MAX, std::memory_order_acquire))
        {
            val = 0;
        }
        else
        {
            return;
        }
    }

    while (!mLock.compare_exchange_weak(val, INT32_MAX, std::memory_order_acquire))
    {
        std::this_thread::yield();
        val = 0;
    }
}



/*-------------------------------------
 * Attempt to lock
-------------------------------------*/
inline bool Futex::try_lock() noexcept
{
    return mLock.exchange(INT32_MAX, std::memory_order_acq_rel) == 0;
}



/*-------------------------------------
 * Futex unlock
-------------------------------------*/
inline void Futex::unlock() noexcept
{
    mLock.store(0, std::memory_order_release);
}



} // end utils namespace
} // end ls namespace
