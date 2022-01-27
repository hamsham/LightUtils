
#include <chrono>
#include <limits>

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/RandomNum.h"



namespace ls
{
namespace utils
{



/*-------------------------------------
 * Seeded Constructor
 * ----------------------------------*/
RandomNum::RandomNum(uint32_t s) noexcept
{
    this->seed(s);
}



/*-------------------------------------
 * Default Constructor
 * ----------------------------------*/
RandomNum::RandomNum() noexcept
{
    this->seed();
}



/*-------------------------------------
 * Copy Constructor
 * ----------------------------------*/
RandomNum::RandomNum(const RandomNum& rn) noexcept :
    mIndex{rn.mIndex}
{
    for (unsigned i = 0; i < 16; ++i)
    {
        mState[i] = rn.mState[i];
    }
}



/*-------------------------------------
 * Move Constructor
 * ----------------------------------*/
RandomNum::RandomNum(RandomNum&& rn) noexcept :
    mIndex{rn.mIndex}
{
    for (unsigned i = 0; i < 16; ++i)
    {
        mState[i] = rn.mState[i];
    }
}



/*-------------------------------------
 * Copy Operator
 * ----------------------------------*/
RandomNum& RandomNum::operator=(const RandomNum& rn) noexcept
{
    if (&rn != this)
    {
        mIndex = rn.mIndex;

        for (unsigned i = 0; i < 16; ++i)
        {
            mState[i] = rn.mState[i];
        }
    }

    return *this;
}



/*-------------------------------------
 * Move Operator
 * ----------------------------------*/
RandomNum& RandomNum::operator=(RandomNum&& rn) noexcept
{
    if (&rn != this)
    {
        mIndex = rn.mIndex;

        for (unsigned i = 0; i < 16; ++i)
        {
            mState[i] = rn.mState[i];
        }
    }

    return *this;
}



/*-------------------------------------
 * Initialize the random distribution
 * ----------------------------------*/
void RandomNum::seed(uint32_t s) noexcept
{
    for (unsigned int i = 0; i < 16; ++i)
    {
        mState[i] = s++;
        this->operator()(); // initializing the mState to random bits
    }

    mIndex = 0;
}



/*-------------------------------------
 * Default random distribution initialization
 * ----------------------------------*/
void RandomNum::seed() noexcept
{
    this->seed((uint32_t)std::chrono::system_clock::now().time_since_epoch().count());
}



/*-------------------------------------
 * Generate a random number
 * ----------------------------------*/
uint32_t RandomNum::operator()() noexcept
{
    uint32_t a, b, c, d;
    a = mState[mIndex];
    c = mState[(mIndex + 13) & 15];
    b = a ^ c ^ (a << 16) ^ (c << 15);
    c = mState[(mIndex + 9) & 15];
    c ^= (c >> 11);
    a = mState[mIndex] = b ^ c;
    d = a ^ ((a << 5) & 0xDA442D24);
    mIndex = (mIndex + 15) & 15;
    a = mState[mIndex];
    mState[mIndex] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);

    return mState[mIndex];
}



/*-------------------------------------
 * Generate a random float within an inclusive range
 * ----------------------------------*/
float RandomNum::randRangeF(const float low, const float high) noexcept
{
    LS_DEBUG_ASSERT(low < high);

    const float delta = high - low;
    const float fmax = (float)std::numeric_limits<unsigned long>::max();

    return low + ((float)this->operator()() / (fmax / delta));
}



/*-------------------------------------
 * Generate a random int within an inclusive range.
 * ----------------------------------*/
int RandomNum::randRangeI(const int low, const int high) noexcept
{
    LS_DEBUG_ASSERT(low < high);

    const int delta = (high+1) - low;

    return low + ((int)this->operator()() % delta);
}



/*-------------------------------------
 * Generate a random unsigned int within an inclusive range.
 * ----------------------------------*/
unsigned RandomNum::randRangeU(const unsigned low, const unsigned high) noexcept
{
    LS_DEBUG_ASSERT(low < high);

    const unsigned delta = (high+1) - low;

    return low + ((unsigned)this->operator()() % delta);
}



} // end utils namespace
} // end ls namespace
