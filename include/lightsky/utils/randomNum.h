
#ifndef __LS_UTILS_RANDON_H__
#define __LS_UTILS_RANDON_H__

namespace ls {
namespace utils {

/**
 *  @brief Pseudo-randomn number generator using the WELLRNC 512 algorithm.
 *  
 *  The implementation was adapted from:
 *  http://www.lomont.org/Math/Papers/Papers.php
 *  (see: Random Number Generation)
 *  http://www.lomont.org/Math/Papers/2008/Lomont_PRNG_2008.pdf
 */
class randomNum {
    private:
        /**
         *  @brief state
         *  A set of seeds that are used to extend the period of random
         *  numbers.
         */
        unsigned long state[16] = {0};
        
        /**
         *  @brief index
         *  An iterator that's used to extract random seeds from 'state'.
         */
        unsigned int index = 0;
        
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
        randomNum(unsigned long s);
        
        /**
         *  @brief Constructor
         *  Initializes a non-seeded PRNG.
         */
        randomNum();
        
        /**
         *  @brief Copy Constructor
         *  Copies all data from the input parameter into *this.
         *  
         *  @param prng
         *  A constant reference to another pseudo-random number generator.
         */
        randomNum(const randomNum& prng);
        
        /**
         *  @brief Move Constructor
         *  Copies all data from the input parameter into *this.
         *  
         *  @param prng
         *  An r-value reference to another pseudo-random number generator.
         */
        randomNum(randomNum&& prng);
        
        /**
         *  @brief Destructor
         *  Frees all resources used by this.
         */
        ~randomNum();
        
        /**
         *  @brief Copy Operator
         *  Copies all data from the input parameter into *this.
         *  
         *  @param prng
         *  An r-value reference to another pseudo-random number generator.
         *  
         *  @return a reference to *this.
         */
        randomNum& operator=(const randomNum&);
        
        /**
         *  @brief Move Operator
         *  Copies all data from the input parameter into *this.
         *  
         *  @param prng
         *  An r-value reference to another pseudo-random number generator.
         *  
         *  @return a reference to *this.
         */
        randomNum& operator=(randomNum&&);
        
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
        void seed(unsigned long s);
        
        /**
         *  @brief Seed
         *  Seeds the PRNG in order to provide a new set of pseudo-random
         *  numbers.
         *  
         *  This method uses the current system time in order to seed the
         *  internal PRNG algorithm.
         */
        void seed();
        
        /**
         *  @brief Overloaded function operator.
         *  Generates a pseudo-random number using the WELL-512 PRNG algorithm.
         *  
         *  @return a randomly generated unsigned integral type.
         */
        unsigned long operator() ();

        /*-------------------------------------
            Helper functions
        -------------------------------------*/
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
        static float randRangeF(randomNum& prng, float low, float high);

        /**
         *  @brief randRangeF
         *  Generates a random integral value in between two specified values
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
         *  @return a pseudo-random integer value clamped in between 'low' and 'high.'
         */
        static int randRangeI(randomNum& prng, int low, int high);
};

} // end utils namespace
} // end ls namespace

#endif /* __LS_UTILS_RANDON_H__ */