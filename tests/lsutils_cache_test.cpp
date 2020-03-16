
#include <iostream>

#include "lightsky/utils/LRUCache.hpp"
#include "lightsky/utils/IndexedCache.hpp"
#include "lightsky/utils/StringUtils.h" // extra double precision with utils::to_string()
#include "lightsky/utils/Time.hpp"

// Explicit template instantiation to help catch compiler errors.
constexpr size_t cacheSize = 96;
template class ls::utils::LRUCache<std::string, cacheSize>;
template class ls::utils::IndexedCache<std::string, cacheSize>;

using TestCacheLRU = ls::utils::LRUCache<std::string, cacheSize>;
using TestCacheIndexed = ls::utils::IndexedCache<std::string, cacheSize>;



template <typename CacheType>
size_t test_hash(size_t totalElems, size_t indexModifier = 100)
{
    CacheType cache;
    size_t hits = 0;

    for (unsigned i = 0; i < totalElems; ++i)
    {
        std::string j;

        //std::cout << "Caching " << i << ": ";

        const std::string* cachedVal = cache.query_or_update(i % indexModifier, j, [](size_t id)->std::string { return ls::utils::to_str(id); });

        if (cachedVal)
        {
            ++hits;
            //std::cout << "hit";
        }
        /*
        else
        {
            std::cout << "miss";
        }

        std::cout << ' ' << j << std::endl;
        */
    }

    return hits;
}



int main()
{
    constexpr size_t totalElems = 4096;//1 << 24;
    ls::utils::Clock<double> timer;

    timer.stop();
    timer.start();
    const size_t lruHits = test_hash<TestCacheLRU>(totalElems);
    timer.tick();

    const double lruHitRatio = 100.0 * ((double)lruHits / (double)totalElems);
    std::cout << "LRU Cache Hits (%): " << ls::utils::to_str(lruHitRatio) << " (" << ls::utils::to_str(timer.tick_time().count()) << "s)." << std::endl;

    timer.stop();
    timer.start();
    const size_t idxHits = test_hash<TestCacheIndexed>(totalElems);
    timer.tick();
    const double idxHitRatio = 100.0 * ((double)idxHits / (double)totalElems);
    std::cout << "Indexed Cache Hits (%): " << ls::utils::to_str(idxHitRatio) << " (" << ls::utils::to_str(timer.tick_time().count()) << "s)." << std::endl;

    return 0;
}
