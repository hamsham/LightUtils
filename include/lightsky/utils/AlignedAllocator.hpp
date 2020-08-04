
#ifndef LS_UTILS_ALIGNED_ALLOCATOR_HPP
#define LS_UTILS_ALIGNED_ALLOCATOR_HPP

#include <cstddef> // std::size_t std::ptrdiff_t
#include <type_traits> // std::integral_constant

namespace ls
{
namespace utils
{



/**----------------------------------------------------------------------------
 * @brief The AlignedAllocator class is an std::allocator-compatible object
 * which attempts to create/free allocations using the system DWORD size. On
 * systems supporting SIMD instructions, the allocations will be aligned to
 * the largest SIMD-type offered by the system.
----------------------------------------------------------------------------*/
template <typename T>
class AlignedAllocator
{
  private:
    template<typename U> struct _RebindType { typedef AlignedAllocator<U> other; };

  public:
    typedef T value_type;

    typedef T* pointer;

    typedef const T* const_pointer;

    typedef T& reference;

    typedef const T& const_reference;

    typedef std::size_t size_type;

    typedef std::ptrdiff_t difference_type;

    typedef std::integral_constant<bool, true> propagate_on_container_move_assignment;

    template <typename U>
    using rebind = _RebindType<U>;

    typedef std::integral_constant<bool, true> is_always_equal;

  public:
    ~AlignedAllocator() noexcept = default;

    constexpr AlignedAllocator() noexcept;

    constexpr AlignedAllocator(const AlignedAllocator& allocator) noexcept;

    //AlignedAllocator(AlignedAllocator&& allocator) noexcept;

    template <typename U>
    constexpr AlignedAllocator(const AlignedAllocator<U>& allocator) noexcept;

    //template <typename U>
    //AlignedAllocator(AlignedAllocator<U>&& allocator) noexcept;

    pointer address(reference x) const noexcept;

    const_pointer address(const_reference x) const noexcept;

    pointer allocate(size_type n) noexcept;

    pointer allocate(size_type n, const void* hint) noexcept;

    void deallocate(pointer p, std::size_t n);

    constexpr size_type max_size() const noexcept;

    void construct(pointer p, const_reference  val);

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args);

    void destroy(pointer p);

    template <typename U>
    void destroy(U* p);
};



template <typename T1, typename T2>
constexpr bool operator==(const AlignedAllocator<T1>& lhs, const AlignedAllocator<T2>& rhs) noexcept;

template <typename T1, typename T2>
constexpr bool operator!=(const AlignedAllocator<T1>& lhs, const AlignedAllocator<T2>& rhs) noexcept;



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/AlignedAllocatorImpl.hpp"

#endif /* LS_UTILS_ALIGNED_ALLOCATOR_HPP */
