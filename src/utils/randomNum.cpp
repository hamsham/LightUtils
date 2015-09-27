
#include <chrono>
#include <limits>

#include "lightsky/utils/assertions.h"
#include "lightsky/utils/randomNum.h"

namespace ls {
namespace utils {

/*-------------------------------------
 * Seeded Constructor
 * ----------------------------------*/
randomNum::randomNum(const unsigned long s)
 {
    this->seed(s);
}

/*-------------------------------------
 * Default Constructor
 * ----------------------------------*/
randomNum::randomNum()
{
}

/*-------------------------------------
 * Copy Constructor
 * ----------------------------------*/
randomNum::randomNum(const randomNum& rn)
:
    index{rn.index}
{
    for (unsigned i = 0; i < 16; ++i)
    {
        state[i] = rn.state[i];
    }
}

/*-------------------------------------
 * Move Constructor
 * ----------------------------------*/
randomNum::randomNum(randomNum&& rn)
:
    index{rn.index}
{
    for (unsigned i = 0; i < 16; ++i)
    {
        state[i] = rn.state[i];
    }
}

/*-------------------------------------
 * Destructor
 * ----------------------------------*/
randomNum::~randomNum()
{}

/*-------------------------------------
 * Copy Operator
 * ----------------------------------*/
randomNum& randomNum::operator=(const randomNum& rn)
{
    index = rn.index;

    for (unsigned i = 0; i < 16; ++i)
    {
        state[i] = rn.state[i];
    }

    return *this;
}

/*-------------------------------------
 * Move Operator
 * ----------------------------------*/
randomNum& randomNum::operator=(randomNum&& rn)
{
    index = rn.index;

    for (unsigned i = 0; i < 16; ++i)
    {
        state[i] = rn.state[i];
    }

    return *this;
}

/*-------------------------------------
 * Initialize the random distribution
 * ----------------------------------*/
void randomNum::seed(unsigned long s)
{
    for (unsigned int i = 0; i < 16; ++i)
    {
        state[i] = s++;
        this->operator()(); // initializing the state to random bits
    }

    index = 0;
}

/*-------------------------------------
 * Default random distribution initialization
 * ----------------------------------*/
void randomNum::seed()
{
    this->seed((long unsigned)std::chrono::system_clock::now().time_since_epoch().count());
}

/*-------------------------------------
 * Generate a random number
 * ----------------------------------*/
unsigned long randomNum::operator()()
{
    unsigned long a, b, c, d;
    a = state[index];
    c = state[(index+13)&15];
    b = a^c^(a<<16)^(c<<15);
    c = state[(index+9)&15];
    c ^= (c>>11);
    a = state[index] = b^c;
    d = a^((a<<5)&0xDA442D24UL);
    index = (index + 15)&15;
    a = state[index];
    state[index] = a^b^d^(a<<2)^(b<<18)^(c<<28);

    return state[index];
}

/*-------------------------------------
 * Generate a random float within a an inclusive range
 * ----------------------------------*/
float randomNum::randRangeF(randomNum& prng, const float low, const float high)
{
    LS_DEBUG_ASSERT(low < high);

    const float delta = high - low;
    const float fmax = (float)std::numeric_limits<unsigned long>::max();

    return low + ((float)prng() / (fmax / delta));
}

/*-------------------------------------
 * Generate a random int within an inclusive range.
 * ----------------------------------*/
int randomNum::randRangeI(randomNum& prng, const int low, const int high) {
    LS_DEBUG_ASSERT(low < high);

    return static_cast<int>(randRangeF(prng, static_cast<float>(low), static_cast<float>(high)));
}

} // end utils namespace
} // end hamlibs namespace
