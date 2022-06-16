
#ifndef LS_UTILS_TUPLE_H
#define LS_UTILS_TUPLE_H

#include <limits>
#include <new>
#include <type_traits>
#include <utility>

#include "lightsky/setup/Api.h"

#include "lightsky/utils/ByteSize.h"



namespace ls
{
namespace utils
{



/**----------------------------------------------------------------------------
 * @brief Helper class for indexing the types within a Tuple
-----------------------------------------------------------------------------*/
template <unsigned N, typename T0, typename... T>
struct TupleElement;



template <typename T>
struct TupleElement<0u, T>
{
    typedef T type;

    static constexpr unsigned offset()
    {
        return 0;
    }
};



template <typename T0, typename... T>
struct TupleElement<0u, T0, T...>
{
    typedef T0 type;

    static constexpr unsigned offset()
    {
        return 0;
    }
};



template <unsigned N, typename T0, typename... T>
struct TupleElement
{
    typedef typename TupleElement<N-1u, T...>::type type;

    static constexpr unsigned offset()
    {
        return sizeof(T0) + TupleElement<N-1u, T...>::offset();
    }
};



/**----------------------------------------------------------------------------
 * @brief Helper class for locating the types within a Tuple
-----------------------------------------------------------------------------*/
template <typename R, typename T0, typename... T>
struct TupleMatcher;



template <typename R>
struct TupleMatcher<R, R>
{
    typedef R type;

    static constexpr unsigned offset()
    {
        return 0;
    }
};



template <typename R, typename... T>
struct TupleMatcher<R, R, T...>
{
    typedef R type;

    static constexpr unsigned offset()
    {
        return 0;
    }
};



template <typename R, typename T0, typename... T>
struct TupleMatcher
{
    typedef typename TupleMatcher<R, T...>::type type;

    static constexpr unsigned offset()
    {
        return sizeof(T0) + TupleMatcher<R, T...>::offset();
    }
};



/**----------------------------------------------------------------------------
 * @brief Helper class for indexing the types within a Tuple at runtime
-----------------------------------------------------------------------------*/
template <typename... data_t>
struct TupleIndexer
{
    template<typename arg_t>
    static constexpr unsigned offset(unsigned index, unsigned numBytes, arg_t*)
    {
        return (index == 0) ? numBytes : std::numeric_limits<unsigned>::max();
    }



    template<typename arg_t, typename... args_t>
    static constexpr unsigned offset(unsigned index, unsigned numBytes, arg_t*, args_t* ...)
    {
        return (index == 0)
               ? numBytes
               : offset<args_t...>(index - 1, sizeof(arg_t) + numBytes, ((args_t * )nullptr)...);
    }



    static constexpr unsigned offset(unsigned index)
    {
        return offset<data_t...>(index, 0, ((data_t*)nullptr)...);
    }
};




/**----------------------------------------------------------------------------
 *  @brief Tuple Class
 *  An tuple is a set of objects determined at compile-time. These objects
 *  are constructed within a tightly packed buffer. It is recommended to
 *  construct a tuple using the "new" operator if there are many objects.
 * --------------------------------------------------------------------------*/
template<typename... data_t>
class Tuple
{
  private:
    /**
     *  @brief Construct an object at a preallocated space within a buffer
     *
     *  @note
     *  The buffer must be preallocated. The pointer passed into the function
     *  is a sentinel value (for the sake of recursion) and is not used for
     *  anything.
     */
    template<typename arg_t>
    static void construct_objs(char* buffer, unsigned offset, const arg_t*);

    /**
     *  @brief Construct an object at a preallocated space within a buffer
     *
     *  @note
     *  The buffer must be preallocated. The pointers passed into the function
     *  are sentinel values (for the sake of recursion) and are not used for
     *  anything.
     */
    template<typename arg_t, typename... args_t>
    static void construct_objs(char* buffer, unsigned offset, const arg_t*, const args_t*... args);

    /**
     *  @brief Copy construct an object at a preallocated space within a buffer
     *
     *  @note
     *  The buffer must be preallocated. The pointer passed into the function
     *  is a sentinel value (for the sake of recursion) and is not used for
     *  anything.
     */
    template<typename arg_t>
    static void construct_objs(char* buffer, unsigned offset, arg_t&& arg);

    /**
     *  @brief Copy construct an object at a preallocated space within a buffer
     *
     *  @note
     *  The buffer must be preallocated. The pointers passed into the function
     *  are sentinel values (for the sake of recursion) and are not used for
     *  anything.
     */
    template<typename arg_t, typename... args_t>
    static void construct_objs(char* buffer, unsigned offset, arg_t&& arg, args_t&&... args);

    /**
     *  @brief Destroy an object at a preallocated space within a buffer.
     */
    template<typename arg_t>
    static void destroy_objs(char* buffer, unsigned offset, arg_t*);

    /**
     *  @brief Destroy an object at a preallocated space within a buffer.
     */
    template<typename arg_t, typename... args_t>
    static void destroy_objs(char* buffer, unsigned offset, arg_t*, args_t* ... args);

    /*-------------------------------------------------------------------------
     * Movements and copies
     * ----------------------------------------------------------------------*/
    /**
     *  @brief Copy an object at a preallocated space within a buffer
     *
     *  @note
     *  The buffer must be preallocated. The pointer passed into the function
     *  is a sentinel value (for the sake of recursion) and is not used for
     *  anything.
     */
    template<unsigned index, typename arg_t>
    static void copy_objs(const Tuple<data_t...>&, char* buffer, unsigned offset, arg_t*);

    /**
     *  @brief Construct an object at a preallocated space within a buffer
     *
     *  @note
     *  The buffer must be preallocated. The pointers passed into the function
     *  are sentinel values (for the sake of recursion) and are not used for
     *  anything.
     */
    template<unsigned index, typename arg_t, typename... args_t>
    static void copy_objs(const Tuple<data_t...>&, char* buffer, unsigned offset, arg_t*, args_t* ... args);

    /**
     *  @brief Move an object at a preallocated space within a buffer.
     */
    template<unsigned index, typename arg_t>
    static void move_objs(Tuple<data_t...>&&, char* buffer, unsigned offset, arg_t*);

    /**
     *  @brief Move an object at a preallocated space within a buffer.
     */
    template<unsigned index, typename arg_t, typename... args_t>
    static void move_objs(Tuple<data_t...>&&, char* buffer, unsigned offset, arg_t*, args_t* ... args);

  private:
    /**
     *  @brief mBuffer
     *
     *  A buffer of bytes (characters) that is used to hold objects of an
     *  tuple. This allows objects to be added at compile-time and
     *  constructed in a tightly packed array.
     */
    char mBuffer[NumBytes<data_t...>::value()];

  public:
    /**
     *  @brief Constructor
     *
     *  Initialize all parameterized data types within a byte array.
     */
    Tuple() noexcept;

    /**
     *  @brief Copy Constructor
     *
     *  Uses each object's copy operator to copy data into *this.
     */
    Tuple(data_t&&...) noexcept;

    /**
     *  @brief Copy Constructor
     *
     *  Uses each object's copy operator to copy data into *this.
     */
    Tuple(const Tuple&) noexcept;

    /**
     *  @brief Move Constructor
     *
     *  Uses each object's move operator to move data into *this.
     */
    Tuple(Tuple&&) noexcept;

    /**
     *  @brief Destructor
     *
     *  Calls the destructor on all member objects requested at
     *  compile-time.
     */
    ~Tuple() noexcept;

    /**
     *  @brief Copy Operator
     *
     *  Uses each object's copy operator to copy data into *this.
     */
    Tuple& operator=(const Tuple&) noexcept;

    /**
     *  @brief Move Operator
     *
     *  Uses each object's move operator to move data into *this.
     */
    Tuple& operator=(Tuple&&) noexcept;

    /*---------------------------------------------------------------------
     * Constant Methods
     * ------------------------------------------------------------------*/
    /**
     *  @brief Get a constant object contained within *this. This object is
     *  specified using template parameters.
     *
     *  @return A constant reference to the first object of a specific type.
     */
    template<typename request_t>
    const request_t& first_of() const noexcept;

    /*---------------------------------------------------------------------
     * Non-Constant versions of methods
     * ------------------------------------------------------------------*/
    /**
     *  @brief Get an object contained within *this. This object is
     *  specified using a template parameter.
     *
     *  @return A reference to the first object of a specific type.
     */
    template<typename request_t>
    request_t& first_of() noexcept;

    /**
     *  @brief Get an object contained within *this. This object is
     *  specified using a template parameter.
     *
     *  @return A constant reference to an object contained within *this.
     */
    template<unsigned index>
    const typename TupleElement<index, data_t...>::type& const_element() const noexcept;

    /**
     *  @brief Get an object contained within *this. This object is
     *  specified using a template parameter.
     *
     *  @return A constant reference to an object contained within *this.
     */
    template<unsigned index>
    typename TupleElement<index, data_t...>::type& element() noexcept;

    /**
     *  @brief Get a constant object contained within *this. This object is
     *  specified using an array-like index.
     *
     *  @param index
     *  An array-like index that can be used to retrieve a value.
     *
     *  @return
     *  A constant pointer to a tuple object if it exists. Returns
     *  otherwise.
     */
    const void* get(unsigned index) const noexcept;

    /**
     *  @brief Get an object contained within *this. This object is
     *  specified using an array-like index.
     *
     *  @param index
     *  An array-like index that can be used to retrieve a value.
     *
     *  @return
     *  A pointer to a tuple object if it exists. Returns NULL otherwise.
     */
    void* get(unsigned index) noexcept;

    /**
     *  @brief Retrieve the number of objects contained within a tuple.
     *
     *  @return
     *  An unsigned integral type representing the number of objects that
     *  are stored in *this.
     */
    unsigned size() const noexcept;
};



/*-----------------------------------------------------------------------------
 * Static Tuple Methods
 * --------------------------------------------------------------------------*/
/*-------------------------------------
 * tuple_t<data_t...>::construct (Sentinel)
 * ----------------------------------*/
template<typename... data_t>
template<typename arg_t>
inline void Tuple<data_t...>::construct_objs(char* buffer, unsigned offset, const arg_t*)
{
    new(buffer + offset) arg_t{};
}



/*-------------------------------------
 * tuple_t<data_t...>::construct
 * ----------------------------------*/
template<typename... data_t>
template<typename arg_t, typename... args_t>
inline void Tuple<data_t...>::construct_objs(char* buffer, unsigned offset, const arg_t*, const args_t*... args)
{
    new(buffer + offset) arg_t{};
    construct_objs<args_t...>(buffer, offset + sizeof(arg_t), args...);
}



/*-------------------------------------
 * tuple_t<data_t...>::construct (Sentinel)
 * ----------------------------------*/
template<typename... data_t>
template<typename arg_t>
inline void Tuple<data_t...>::construct_objs(char* buffer, unsigned offset, arg_t&& arg)
{
    new(buffer + offset) arg_t{std::forward<arg_t>(arg)};
}



/*-------------------------------------
 * tuple_t<data_t...>::construct
 * ----------------------------------*/
template<typename... data_t>
template<typename arg_t, typename... args_t>
inline void Tuple<data_t...>::construct_objs(char* buffer, unsigned offset, arg_t&& arg, args_t&&... args)
{
    new(buffer + offset) arg_t{std::forward<arg_t>(arg)};
    construct_objs<args_t...>(buffer, offset + sizeof(arg_t), std::forward<args_t>(args)...);
}



/*-------------------------------------
 * tuple_t<data_t...>::destroy
 * ----------------------------------*/
template<typename... data_t>
template<typename arg_t>
inline void Tuple<data_t...>::destroy_objs(char* buffer, unsigned offset, arg_t*)
{
    if (std::is_destructible<arg_t>())
    {
        reinterpret_cast<arg_t*> (buffer + offset)->~arg_t();
    }
}



/*-------------------------------------
 * tuple_t<data_t...>::destroy
 * ----------------------------------*/
template<typename... data_t>
template<typename arg_t, typename... args_t>
inline void Tuple<data_t...>::destroy_objs(char* buffer, unsigned offset, arg_t*, args_t* ... args)
{
    if (std::is_destructible<arg_t>())
    {
        reinterpret_cast<arg_t*> (buffer + offset)->~arg_t();
    }

    destroy_objs<args_t...>(buffer, offset + sizeof(arg_t), args...);
}



/*-----------------------------------------------------------------------------
 * Movement Methods
 * --------------------------------------------------------------------------*/
/*-------------------------------------
 * tuple_t<data_t...>::copyObjects (Sentinel)
 * ----------------------------------*/
template<typename... data_t>
template<unsigned index, typename arg_t>
inline void Tuple<data_t...>::copy_objs(const Tuple<data_t...>& aggregate, char* buffer, unsigned offset, arg_t*)
{
    static_assert(std::is_copy_assignable<arg_t>(), "Aggregated objects must have a copy operator available.");

    arg_t* pArg = (arg_t*)(buffer + offset);
    *pArg = aggregate.const_element<index>();
}



/*-------------------------------------
 * tuple_t<data_t...>::copyObjects
 * ----------------------------------*/
template<typename... data_t>
template<unsigned index, typename arg_t, typename... args_t>
inline void Tuple<data_t...>::copy_objs(const Tuple<data_t...>& aggregate, char* buffer, unsigned offset, arg_t*, args_t* ... args)
{
    static_assert(std::is_copy_assignable<arg_t>(), "Aggregated objects must have a copy operator available.");

    arg_t* pArg = (arg_t*)(buffer + offset);
    *pArg = aggregate.const_element<index>();

    copy_objs<index+1, args_t...>(aggregate, buffer, offset + sizeof(arg_t), args...);
}



/*-------------------------------------
 * tuple_t<data_t...>::moveObjects (Sentinel)
 * ----------------------------------*/
template<typename... data_t>
template<unsigned index, typename arg_t>
inline void Tuple<data_t...>::move_objs(Tuple<data_t...>&& aggregate, char* buffer, unsigned offset, arg_t*)
{
    static_assert(std::is_copy_assignable<arg_t>(), "Aggregated objects must have a move operator available.");

    arg_t* pArg = (arg_t*)(buffer + offset);

    *pArg = std::move(aggregate.element<index>());
}



/*-------------------------------------
 * tuple_t<data_t...>::moveObjects
 * ----------------------------------*/
template<typename... data_t>
template<unsigned index, typename arg_t, typename... args_t>
inline void Tuple<data_t...>::move_objs(Tuple<data_t...>&& aggregate, char* buffer, unsigned offset, arg_t*, args_t* ... args)
{
    static_assert(std::is_copy_assignable<arg_t>(), "Aggregated objects must have a move operator available.");

    arg_t* pArg = (arg_t*)(buffer + offset);
    *pArg = std::move(aggregate.element<index>());

    move_objs<index+1, args_t...>(std::forward<Tuple<data_t...>>(aggregate), buffer, offset + sizeof(arg_t), args...);
}



/*-----------------------------------------------------------------------------
 * Non-Static Tuple Methods
 * --------------------------------------------------------------------------*/
/*-------------------------------------
 * Constructor
 * ----------------------------------*/
template<typename... data_t>
inline Tuple<data_t...>::Tuple() noexcept
{
    construct_objs<data_t...>(mBuffer, 0, ((data_t*)nullptr)...);
}



/*-------------------------------------
 * Copy Constructor
 * ----------------------------------*/
template<typename... data_t>
inline Tuple<data_t...>::Tuple(data_t&&... data) noexcept
{
    construct_objs<data_t...>(mBuffer, 0, std::forward<data_t>(data)...);
}



/*-------------------------------------
 * Copy Constructor
 * ----------------------------------*/
template<typename... data_t>
inline Tuple<data_t...>::Tuple(const Tuple& a) noexcept :
    Tuple{}
{
    copy_objs<0, data_t...>(a, mBuffer, 0, ((data_t*)nullptr)...);
}



/*-------------------------------------
 * Move Constructor
 * ----------------------------------*/
template<typename... data_t>
inline Tuple<data_t...>::Tuple(Tuple&& a) noexcept :
    Tuple{}
{
    move_objs<0, data_t...>(std::forward<Tuple < data_t...>>(a), mBuffer, 0, ((data_t*)nullptr)...);
}



/*-------------------------------------
 * Destructor
 * ----------------------------------*/
template<typename... data_t>
inline Tuple<data_t...>::~Tuple() noexcept
{
    destroy_objs(mBuffer, 0, ((data_t*)nullptr)...);
}



/*-------------------------------------
 * Copy Operator
 * ----------------------------------*/
template<typename... data_t>
inline Tuple<data_t...>& Tuple<data_t...>::operator=(const Tuple& a) noexcept
{
    copy_objs<0, data_t...>(a, mBuffer, 0, ((data_t*)nullptr)...);
    return *this;
}



/*-------------------------------------
 * Move Operator
 * ----------------------------------*/
template<typename... data_t>
inline Tuple<data_t...>& Tuple<data_t...>::operator=(Tuple&& a) noexcept
{
    move_objs<0, data_t...>(std::forward<Tuple < data_t...>>(a), mBuffer, 0, ((data_t*)nullptr)...);
    return *this;
}



/*-----------------------------------------------------------------------------
 * Get an object using an its type
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Get an object contained within *this by deducing the first matching template
 * parameter (const).
 * ----------------------------------*/
template<typename... data_t>
template<typename request_t>
inline LS_INLINE const request_t& Tuple<data_t...>::first_of() const noexcept
{
    return *reinterpret_cast<const typename TupleMatcher<request_t, data_t...>::type*>((unsigned long long)mBuffer + TupleMatcher<request_t, data_t...>::offset());
}



/*-------------------------------------
 * Get an object contained within *this by deducing the first matching template
 * parameter.
 * ----------------------------------*/
template<typename... data_t>
template<typename request_t>
inline request_t& Tuple<data_t...>::first_of() noexcept
{
    return *reinterpret_cast<typename TupleMatcher<request_t, data_t...>::type*>((unsigned long long)mBuffer + TupleMatcher<request_t, data_t...>::offset());
}



/*-------------------------------------
 * Get an object using its index (const)
 * ----------------------------------*/
template<typename... data_t>
template<unsigned index>
inline LS_INLINE const typename TupleElement<index, data_t...>::type& Tuple<data_t...>::const_element() const noexcept
{
    return *reinterpret_cast<const typename TupleElement<index, data_t...>::type*>((unsigned long long)mBuffer + TupleElement<index, data_t...>::offset());
}



/*-------------------------------------
 * Get an object using its index
 * ----------------------------------*/
template<typename... data_t>
template<unsigned index>
inline typename TupleElement<index, data_t...>::type& Tuple<data_t...>::element() noexcept
{
    return *reinterpret_cast<typename TupleElement<index, data_t...>::type*>((unsigned long long)mBuffer + TupleElement<index, data_t...>::offset());
}



/*-----------------------------------------------------------------------------
 * Get an object using an its index (const)
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Get an object contained within *this. This object is specified using
 * an array-like index.
 * ----------------------------------*/
template<typename... data_t>
inline LS_INLINE const void* Tuple<data_t...>::get(unsigned index) const noexcept
{
    return reinterpret_cast<const void*>(mBuffer + TupleIndexer<data_t...>::offset(index));
}



/*-------------------------------------
 * Get an object contained within *this. This object is specified using
 * an array-like index.
 * ----------------------------------*/
template<typename... data_t>
inline void* Tuple<data_t...>::get(unsigned index) noexcept
{
    return reinterpret_cast<void*>(mBuffer + TupleIndexer<data_t...>::offset(index));
}



/*-----------------------------------------------------------------------------
 * Non-retrieval methods
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Retrieve the number of objects contained within an tuple_t.
 * ----------------------------------*/
template<typename... data_t>
inline LS_INLINE unsigned Tuple<data_t...>::size() const noexcept
{
    return sizeof...(data_t);
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_TUPLE_H */
