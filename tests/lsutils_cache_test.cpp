
#include <iostream>

#include "lightsky/utils/LRUCache.hpp"
#include "lightsky/utils/LRU8WayCache.hpp"
#include "lightsky/utils/IndexedCache.hpp"
#include "lightsky/utils/RandomNum.h"
#include "lightsky/utils/StringUtils.h" // extra double precision with utils::to_string()
#include "lightsky/utils/Time.hpp"

// Explicit template instantiation to help catch compiler errors.
constexpr size_t CACHE_SIZE = 8;
constexpr bool TEST_LRU_CACHE = true;
constexpr bool TEST_LRU8_CACHE = true;
constexpr bool TEST_INDEXED_CACHE = true;
constexpr unsigned NUM_TEST_RUNS = 1 << 24;
//constexpr unsigned NUM_TEST_RUNS = 128;
constexpr bool VERBOSE_LOGGING = false;

template class ls::utils::LRUCache<std::string, CACHE_SIZE>;
template class ls::utils::LRU8WayCache<std::string>;
template class ls::utils::IndexedCache<std::string, CACHE_SIZE>;

using TestCacheLRU = ls::utils::LRUCache<std::string, CACHE_SIZE>;
using TestCacheLRU8 = ls::utils::LRU8WayCache<std::string>;
using TestCacheIndexed = ls::utils::IndexedCache<std::string, CACHE_SIZE>;



template <typename CacheType>
size_t test_hash(size_t totalElems, size_t indexModifier = 4096)
{
    constexpr unsigned seed = 0xDEADBEEF;
    ls::utils::RandomNum rand512{seed};
    CacheType cache{};
    size_t hits = 0;

    (void)indexModifier;

    for (unsigned i = 0; i < totalElems; ++i)
    {
        unsigned r = rand512.randRangeU(0, 8);
        unsigned val = i + r;
        //unsigned val = (rand512() % (unsigned)indexModifier);
        //val = val + (3 - (val % 3));
        std::string* j;

        const std::string* cachedVal = cache.query_or_update(val, &j, [](size_t id)->std::string {
            return ls::utils::to_str(id);
        });

        hits += cachedVal != nullptr;

        if (VERBOSE_LOGGING)
        {
            std::cout << "Caching " << i << ": ";

            if (cachedVal)
            {
                std::cout << "hit";
            }
            else
            {
                std::cout << "miss";
            }

            std::cout << ' ' << j << std::endl;
        }
    }

    return hits;
}



void print_cache_stats(size_t cacheHits, size_t totalElems, const ls::utils::Clock<double, std::chrono::milliseconds::period>& timer, const char* cacheName)
{
    const double lruHitRatio = 100.0 * ((double)cacheHits / (double)totalElems);

    std::cout
        << cacheName
        << "\n\tTotal Hits: " << cacheHits << " / " << totalElems
        << "\n\tRatio (%):  " << ls::utils::to_str(lruHitRatio)
        << "\n\tTime (ms):  " << ls::utils::to_str(timer.tick_time().count())
        << std::endl;
}



int main()
{
    constexpr unsigned totalElems = NUM_TEST_RUNS;
    size_t hits;
    ls::utils::Clock<double, std::chrono::milliseconds::period> timer;

    if (TEST_LRU_CACHE)
    {
        timer.stop();
        timer.start();
        hits = test_hash<TestCacheLRU>(totalElems);
        timer.tick();
        print_cache_stats(hits, totalElems, timer, "LRU");
    }

    if (TEST_LRU8_CACHE)
    {
        timer.stop();
        timer.start();
        hits = test_hash<TestCacheLRU8>(totalElems);
        timer.tick();
        print_cache_stats(hits, totalElems, timer, "LRU (8-Way)");
    }

    if (TEST_INDEXED_CACHE)
    {
        timer.stop();
        timer.start();
        hits = test_hash<TestCacheIndexed>(totalElems);
        timer.tick();
        print_cache_stats(hits, totalElems, timer, "Hash");
    }

    return 0;
}
