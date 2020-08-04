
#include <limits> // std::numeric_limits<>
#include <utility> // std::forward()
#include <memory>

#include "lightsky/utils/Pointer.h"

namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * AlignedAllocator Member Functions
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename T>
constexpr AlignedAllocator<T>::AlignedAllocator() noexcept
{
}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <typename T>
constexpr AlignedAllocator<T>::AlignedAllocator(const AlignedAllocator&) noexcept
{
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
//template <typename T>
//AlignedAllocator<T>::AlignedAllocator(AlignedAllocator&&) noexcept
//{
//}



/*-------------------------------------
 * Copy Constructor (rebound)
-------------------------------------*/
template <typename T>
template <typename U>
constexpr AlignedAllocator<T>::AlignedAllocator(const AlignedAllocator<U>&) noexcept
{
}



/*-------------------------------------
 * Move Constructor (rebound)
-------------------------------------*/
//template <typename T>
//template <typename U>
//AlignedAllocator<T>::AlignedAllocator(AlignedAllocator<U>&&) noexcept
//{
//}



/*-------------------------------------
 * Pointer from a reference
-------------------------------------*/
template <typename T>
typename AlignedAllocator<T>::pointer AlignedAllocator<T>::address(reference x) const noexcept
{
    return &x;
}



/*-------------------------------------
 * Pointer from a referecne (const)
-------------------------------------*/
template <typename T>
typename AlignedAllocator<T>::const_pointer AlignedAllocator<T>::address(const_reference x) const noexcept
{
    return &x;
}



/*-------------------------------------
 * Allocator
-------------------------------------*/
template <typename T>
typename AlignedAllocator<T>::pointer AlignedAllocator<T>::allocate(size_type n) noexcept
{
    return (pointer)ls::utils::aligned_malloc(sizeof(T)*n);
}



/*-------------------------------------
 * Hinted Allocator
-------------------------------------*/
template <typename T>
typename AlignedAllocator<T>::pointer AlignedAllocator<T>::allocate(size_type n, const void* hint) noexcept
{
    (void)hint;
    return (pointer)ls::utils::aligned_malloc(sizeof(T)*n);
}



/*-------------------------------------
 * Deallocate
-------------------------------------*/
template <typename T>
void AlignedAllocator<T>::deallocate(pointer p, std::size_t n)
{
    (void)n;
    ls::utils::aligned_free(p);
}



/*-------------------------------------
 * Maximum allocation size
-------------------------------------*/
template <typename T>
constexpr typename AlignedAllocator<T>::size_type AlignedAllocator<T>::max_size() const noexcept
{
    return std::numeric_limits<std::size_t>::max() / sizeof(T);
}



/*-------------------------------------
 * Object Construction
-------------------------------------*/
template <typename T>
void AlignedAllocator<T>::construct(pointer p, const_reference  val)
{
    ::new((void*)p) T{val};
}



/*-------------------------------------
 * Aggregated construction
-------------------------------------*/
template <typename T>
template <typename U, typename... Args>
void AlignedAllocator<T>::construct(U* p, Args&&... args)
{
    ::new((void*)p) U(std::forward<Args...>(args)...);
}



/*-------------------------------------
 * Object Destruction
-------------------------------------*/
template <typename T>
void AlignedAllocator<T>::destroy(pointer p)
{
    ((pointer)p)->~T();
}



/*-------------------------------------
 * Rebound Destruction
-------------------------------------*/
template <typename T>
template <typename U>
void AlignedAllocator<T>::destroy(U* p)
{
    p->~U();
}



/*-----------------------------------------------------------------------------
 * AlignedAllocator Non-Member Functions
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Compare allocators
-------------------------------------*/
template <typename T1, typename T2>
constexpr bool operator==(const AlignedAllocator<T1>& lhs, const AlignedAllocator<T2>& rhs) noexcept
{
    return &lhs == &rhs;
}



/*-------------------------------------
 * Compare Allocators (not-equal).
-------------------------------------*/
template <typename T1, typename T2>
constexpr bool operator!=(const AlignedAllocator<T1>& lhs, const AlignedAllocator<T2>& rhs) noexcept
{
    return &lhs != &rhs;
}



} // end utils namespace
} // end ls namespace
