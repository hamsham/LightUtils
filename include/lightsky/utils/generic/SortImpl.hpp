
namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Implementations
-----------------------------------------------------------------------------*/
namespace impl
{
/*-------------------------------------
 * Quick Sort
-------------------------------------*/
template <typename data_type, class Comparator>
inline void quick_sort_impl(data_type* const items, const long l, const long r)
{
    constexpr Comparator cmp;
    data_type temp;

    if (r <= l)
    {
        return;
    }

    //const long pivotIndex = (l+r)/2l;
    const long pivotIndex = (l+r) >> 1l;

    temp = items[pivotIndex];
    items[pivotIndex] = items[r];
    items[r] = temp;

    long m = l - 1;
    long n = r;
    const data_type pivot = items[r];

    do
    {
        while (cmp(items[++m], pivot));
        while ((m < n) && cmp(pivot, items[--n]));

        temp     = items[m];
        items[m] = items[n];
        items[n] = temp;
    } while (m < n);

    temp    = items[m];
    items[m] = items[r];
    items[r] = temp;

    quick_sort_impl<data_type, Comparator>(items, l, m - 1);
    quick_sort_impl<data_type, Comparator>(items, m + 1, r);
}



} // end impl namespace
} // end utils namespace



/*-----------------------------------------------------------------------------
 * Invocations
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Quick Sort
-------------------------------------*/
template <typename data_type, class Comparator>
inline void utils::quick_sort(data_type* const items, long count)
{
    impl::quick_sort_impl<data_type, Comparator>(items, 0, count-1);
}




} // end ls namespace
