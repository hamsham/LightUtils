
#ifndef LS_UTILS_RANDON_H
#define LS_UTILS_RANDON_H

#include <cstdint>



namespace ls
{
namespace utils
{



/**
 *  @brief Pseudo-randomn number generator using the WELLRNC 512 algorithm.
 *
 *  The implementation was adapted from:
 *  http://www.lomont.org/Math/Papers/Papers.php
 *  (see: Random Number Generation)
 *  http://www.lomont.org/Math/Papers/2008/Lomont_PRNG_2008.pdf
 */
class RandomNum
{
  private:
    /**
     *  @brief mState
     *  A set of seeds that are used to extend the period of random
     *  numbers.
     */
    uint32_t mState[16] = {0};

    /**
     *  @brief mIndex
     *  An iterator that's used to extract random seeds from 'mState'.
     */
    unsigned mIndex = 0u;

  public:
    /**
     *  @brief Seed Constructor
     *  Seeds the PRNG in order to make random values available immediately
     *  after construction.
     *
     *  @param s
     *  An initial seed value that can be used to initialize the internal
     *  PRNG algorithm. The same seed can be used repeatedly in order to
     *  generate the same set of values.
     */
    explicit RandomNum(uint32_t s) noexcept;

    /**
     *  @brief Constructor
     *  Initializes a non-seeded PRNG.
     */
    RandomNum() noexcept;

    /**
     *  @brief Copy Constructor
     *  Copies all data from the input parameter into *this.
     *
     *  @param prng
     *  A constant reference to another pseudo-random number generator.
     */
    RandomNum(const RandomNum& prng) noexcept;

    /**
     *  @brief Move Constructor
     *  Copies all data from the input parameter into *this.
     *
     *  @param prng
     *  An r-value reference to another pseudo-random number generator.
     */
    RandomNum(RandomNum&& prng) noexcept;

    /**
     *  @brief Destructor
     *  Frees all resources used by this.
     */
    ~RandomNum() = default;

    /**
     *  @brief Copy Operator
     *  Copies all data from the input parameter into *this.
     *
     *  @param prng
     *  An r-value reference to another pseudo-random number generator.
     *
     *  @return a reference to *this.
     */
    RandomNum& operator=(const RandomNum&) noexcept;

    /**
     *  @brief Move Operator
     *  Copies all data from the input parameter into *this.
     *
     *  @param prng
     *  An r-value reference to another pseudo-random number generator.
     *
     *  @return a reference to *this.
     */
    RandomNum& operator=(RandomNum&&) noexcept;

    /**
     *  @brief Seed
     *  Seeds the PRNG in order to provide a new set of pseudo-random
     *  numbers.
     *
     *  @param s
     *  A seed value that can be used to initialize the internal PRNG
     *  algorithm. The same seed can be used repeatedly in order to
     *  generate the same set of values.
     */
    void seed(uint32_t s) noexcept;

    /**
     *  @brief Seed
     *  Seeds the PRNG in order to provide a new set of pseudo-random
     *  numbers.
     *
     *  This method uses the current system time in order to seed the
     *  internal PRNG algorithm.
     */
    void seed() noexcept;

    /**
     *  @brief Overloaded function operator.
     *  Generates a pseudo-random number using the WELL-512 PRNG algorithm.
     *
     *  @return a randomly generated unsigned integral type.
     */
    uint32_t operator()() noexcept;

    /*-------------------------------------
     * Helper functions
     * ----------------------------------*/
    /**
     *  @brief randRangeF
     *  Generates a random floating-point number in between two specified values
     *  (inclusive).
     *
     *  @param prng
     *  A reference to a random-number generator.
     *
     *  @param low
     *  The lower-bound used to clamp a prng value.
     *
     *  @param high
     *  The upper-bound used to clamp a prng value.
     *
     *  @return a pseudo-random floating-point value clamped in between 'low' and
     *  'high.'
     */
    float randRangeF(float low, float high) noexcept;

    /**
     *  @brief randRangeI
     *  Generates a random signed integral value in between two specified
     *  values (inclusive).
     *
     *  @param prng
     *  A reference to a random-number generator.
     *
     *  @param low
     *  The lower-bound used to clamp a prng value.
     *
     *  @param high
     *  The upper-bound used to clamp a prng value.
     *
     *  @return a pseudo-random integer value clamped in between 'low' and 'high.'
     */
    int randRangeI(int low, int high) noexcept;

    /**
     *  @brief randRangeU
     *  Generates a random unsigned integral value in between two specified
     *  values (inclusive).
     *
     *  @param prng
     *  A reference to a random-number generator.
     *
     *  @param low
     *  The lower-bound used to clamp a prng value.
     *
     *  @param high
     *  The upper-bound used to clamp a prng value.
     *
     *  @return a pseudo-random integer value clamped in between 'low' and 'high.'
     */
    unsigned randRangeU(unsigned low, unsigned high) noexcept;
};



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_RANDON_H */
