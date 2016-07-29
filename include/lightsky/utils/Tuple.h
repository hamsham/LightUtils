
#ifndef __LS_UTILS_TUPLE_H__
#define __LS_UTILS_TUPLE_H__

#include <new>
#include <type_traits>
#include <utility>

#include "lightsky/utils/ByteSize.h"

namespace ls {
namespace utils {

/*-----------------------------------------------------------------------------
 * Tuple Object
 * --------------------------------------------------------------------------*/



/**
 *  @brief Tuple Class
 *  An tuple is a set of objects determined at compile-time. These objects
 *  are constructed within a tightly packed buffer. It is recommended to
 *  construct an tuple using the "new" operator if there are many objects.
 */
template <typename... data_t>
class Tuple_t {
    /**
     *  @brief Construct an object at a preallocated space within a buffer
     *
     *  @note
     *  The buffer must be preallocated. The pointer passed into the function
     *  is a sentinel value (for the sake of recursion) and is not used for
     *  anything.
     */
    template <typename arg_t>
    static constexpr
    bool construct_objs(char* buffer, unsigned offset, arg_t*);

    /**
     *  @brief Construct an object at a preallocated space within a buffer
     *
     *  @note
     *  The buffer must be preallocated. The pointers passed into the function
     *  are sentinel values (for the sake of recursion) and are not used for
     *  anything.
     */
    template <typename arg_t, typename... args_t>
    static constexpr
    bool construct_objs(char* buffer, unsigned offset, arg_t*, args_t*... args);

    /**
     *  @brief Destroy an object at a preallocated space within a buffer.
     */
    template <typename arg_t>
    static constexpr
    bool destroy_objs(char* buffer, unsigned offset, arg_t*);

    /**
     *  @brief Destroy an object at a preallocated space within a buffer.
     */
    template <typename arg_t, typename... args_t>
    static inline
    bool destroy_objs(char* buffer, unsigned offset, arg_t*, args_t*... args);

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
    template <typename arg_t>
    static inline
    void copy_objs(const Tuple_t<data_t...>&, char* buffer, unsigned offset, arg_t*);

    /**
     *  @brief Construct an object at a preallocated space within a buffer
     *
     *  @note
     *  The buffer must be preallocated. The pointers passed into the function
     *  are sentinel values (for the sake of recursion) and are not used for
     *  anything.
     */
    template <typename arg_t, typename... args_t>
    static inline
    void copy_objs(const Tuple_t<data_t...>&, char* buffer, unsigned offset, arg_t*, args_t*... args);

    /**
     *  @brief Destroy an object at a preallocated space within a buffer.
     */
    template <typename arg_t>
    static inline
    void move_objs(Tuple_t<data_t...>&&, char* buffer, unsigned offset, arg_t*);

    /**
     *  @brief Destroy an object at a preallocated space within a buffer.
     */
    template <typename arg_t, typename... args_t>
    static inline
    void move_objs(Tuple_t<data_t...>&&, char* buffer, unsigned offset, arg_t*, args_t*... args);

  private:
    /**
     *  @brief Private Constructor
     *
     *  Assists in constructing an tuple at compile-time
     */
    constexpr
    Tuple_t(bool);

    /**
     *  @brief dataBuffer
     *
     *  A buffer of bytes (characters) that is used to hold objects of an
     *  tuple. This allows objects to be added at compile-time and
     *  constructed in a tightly packed array.
     */
    char dataBuffer[get_byte_size<data_t...>()];

    /**
     *  @brief Get the last object in the tuple's byte buffer.
     *
     *  @param offset
     *  A byte offset to the object requested by the template parameter
     *  "request_t".
     *
     *  @param arg_t
     *  A null type pointer, used to help iterate through objects within
     *  an tuple's byte buffer.
     *
     *  @return
     *  A constant pointer to an object object if one exists. Returns NULL
     *  if it was not found.
     */
    template <typename request_t, typename arg_t>
    constexpr
    const request_t* get_obj_at_offset(unsigned offset, arg_t*) const;

    /**
     *  @brief Get an object in the tuple's byte buffer.
     *
     *  @param offset
     *  A byte offset to the object requested by the template parameter
     *  "request_t".
     *
     *  @param arg_t
     *  A null type pointer, used to help iterate through objects within
     *  an tuple's byte buffer.
     *
     *  @param args_t
     *  A set of null pointers, used to help iterate through objects within
     *  an tuple's byte buffer.
     *
     *  @return
     *  A constant pointer to an object object if one exists. Returns NULL
     *  if it was not found.
     */
    template <typename request_t, typename arg_t, typename... args_t>
    constexpr
    const request_t* get_obj_at_offset(unsigned offset, arg_t*, args_t*...) const;

    /**
     *  @brief Get the last object in the tuple's byte buffer.
     *
     *  @param offset
     *  A byte offset to the object requested by the template parameter
     *  "request_t".
     *
     *  @param arg_t
     *  A null type pointer, used to help iterate through objects within
     *  an tuple's byte buffer.
     *
     *  @return
     *  A constant pointer to an object object if one exists. Returns NULL
     *  if it was not found.
     */
    template <typename request_t, typename arg_t>
    inline
    request_t* get_obj_at_offset(unsigned offset, arg_t*);

    /**
     *  @brief Get an object in the tuple's byte buffer.
     *
     *  @param offset
     *  A byte offset to the object requested by the template parameter
     *  "request_t".
     *
     *  @param arg_t
     *  A null type pointer, used to help iterate through objects within
     *  an tuple's byte buffer.
     *
     *  @param args_t
     *  A set of null pointers, used to help iterate through objects within
     *  an tuple's byte buffer.
     *
     *  @return
     *  A constant pointer to an object object if one exists. Returns NULL
     *  if it was not found.
     */
    template <typename request_t, typename arg_t, typename... args_t>
    inline
    request_t* get_obj_at_offset(unsigned offset, arg_t*, args_t*...);

    /**
     *  @brief Get the last object in the data buffer using an index.
     *
     *  @param offset
     *  A byte offset to the object requested by the template parameter
     *  "request_t".
     *
     *  @param arg_t
     *  A null type pointer, used to help iterate through objects within
     *  an tuple's byte buffer.
     *
     *  @return
     *  A constant pointer to an object object if one exists. Returns NULL
     *  if it was not found.
     */
    template <typename arg_t>
    constexpr
    const void* get_obj_at_index(unsigned index, unsigned offset, arg_t*) const;

    /**
     *  @brief Get an tupled object using an index.
     *
     *  @param index
     *  An array index to help determine which object in the tuple
     *  should be retrieved.
     *
     *  @param offset
     *  A byte offset to the object requested from client code.
     *
     *  @param arg_t
     *  A null type pointer, used to help iterate through objects within
     *  an tuple's byte buffer.
     *
     *  @param args_t
     *  A set of null pointers, used to help iterate through objects within
     *  an tuple's byte buffer.
     *
     *  @return
     *  A constant pointer to an object object if one exists. Returns NULL
     *  if it was not found.
     */
    template <typename arg_t, typename... args_t>
    constexpr
    const void* get_obj_at_index(unsigned index, unsigned offset, arg_t*, args_t*...) const;

    /**
     *  @brief Get the last object in the data buffer using an index.
     *
     *  @param offset
     *  A byte offset to the object requested by the template parameter
     *  "request_t".
     *
     *  @param arg_t
     *  A null type pointer, used to help iterate through objects within
     *  an tuple's byte buffer.
     *
     *  @return
     *  A constant pointer to an Object object if one exists. Returns NULL
     *  if it was not found.
     */
    template <typename arg_t>
    inline
    void* get_obj_at_index(unsigned index, unsigned offset, arg_t*);

    /**
     *  @brief Get an tupled object using an index.
     *
     *  @param index
     *  An array index to help determine which object in the tuple
     *  should be retrieved.
     *
     *  @param offset
     *  A byte offset to the object requested from client code.
     *
     *  @param arg_t
     *  A null type pointer, used to help iterate through objects within
     *  an tuple's byte buffer.
     *
     *  @param args_t
     *  A set of null pointers, used to help iterate through objects within
     *  an tuple's byte buffer.
     *
     *  @return
     *  A constant pointer to an object object if one exists. Returns NULL
     *  if it was not found.
     */
    template <typename arg_t, typename... args_t>
    inline
    void* get_obj_at_index(unsigned index, unsigned offset, arg_t*, args_t*...);

  public:
    /**
     *  @brief Constructor
     *
     *  Initialize all parameterized data types within a byte array.
     */
    constexpr
    Tuple_t();

    /**
     *  @brief Copy Constructor
     *
     *  Uses each object's copy operator to copy data into *this.
     */
    Tuple_t(const Tuple_t&);

    /**
     *  @brief Move Constructor
     *
     *  Uses each object's move operator to move data into *this.
     */
    Tuple_t(Tuple_t&&);

    /**
     *  @brief Destructor
     *
     *  Calls the destructor on all member objects requested at
     *  compile-time.
     */
    ~Tuple_t();

    /**
     *  @brief Copy Operator
     *
     *  Uses each object's copy operator to copy data into *this.
     */
    Tuple_t& operator=(const Tuple_t&);

    /**
     *  @brief Move Operator
     *
     *  Uses each object's move operator to move data into *this.
     */
    Tuple_t& operator=(Tuple_t&&);

    /*---------------------------------------------------------------------
     * Constant Methods
     * ------------------------------------------------------------------*/
    /**
     *  @brief Get a constant object contained within *this. This object is
     *  specified using template parameters.
     *
     *  @return
     *  A constant pointer to a tuple object if it exists. Otherwise, this
     *  methods returns NULL.
     */
    template <typename request_t>
    constexpr
    const request_t* get_object() const;

    /*---------------------------------------------------------------------
     * Non-Constant versions of methods
     * ------------------------------------------------------------------*/
    /**
     *  @brief Get an object contained within *this. This object is
     *  specified using a template parameter.
     *
     *  @return a pointer to a tuple object if it exists. Otherwise, this
     *  methods returns NULL.
     */
    template <typename request_t>
    inline
    request_t* get_object();

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
    constexpr
    const void* get_object(unsigned index) const;

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
    inline
    void* get_object(unsigned index);

    /**
     *  @brief Retrieve the number of objects contained within an tuple_t.
     *
     *  @return
     *  An unsigned integral type representing the number of objects that
     *  are stored in *this.
     */
    constexpr
    unsigned get_num_objects() const;
};

/*-------------------------------------
 * Extern Templates for plain data types.
 * ----------------------------------*/
extern template class Tuple_t<signed char>;
extern template class Tuple_t<signed char*>;
extern template class Tuple_t<signed short>;
extern template class Tuple_t<signed short*>;
extern template class Tuple_t<signed int>;
extern template class Tuple_t<signed int*>;
extern template class Tuple_t<signed long>;
extern template class Tuple_t<signed long*>;
extern template class Tuple_t<signed long long>;
extern template class Tuple_t<signed long long*>;

extern template class Tuple_t<unsigned char>;
extern template class Tuple_t<unsigned char*>;
extern template class Tuple_t<unsigned short>;
extern template class Tuple_t<unsigned short*>;
extern template class Tuple_t<unsigned int>;
extern template class Tuple_t<unsigned int*>;
extern template class Tuple_t<unsigned long>;
extern template class Tuple_t<unsigned long*>;
extern template class Tuple_t<unsigned long long>;
extern template class Tuple_t<unsigned long long*>;

extern template class Tuple_t<float>;
extern template class Tuple_t<float*>;
extern template class Tuple_t<double>;
extern template class Tuple_t<double*>;
extern template class Tuple_t<long double>;
extern template class Tuple_t<long double*>;

// data and pointer pairs
extern template class Tuple_t<signed char, signed char*>;
extern template class Tuple_t<signed short, signed short*>;
extern template class Tuple_t<signed int, signed int*>;
extern template class Tuple_t<signed long, signed long*>;
extern template class Tuple_t<signed long long, signed long long*>;

extern template class Tuple_t<unsigned char, unsigned char*>;
extern template class Tuple_t<unsigned short, unsigned short*>;
extern template class Tuple_t<unsigned int, unsigned int*>;
extern template class Tuple_t<unsigned long, unsigned long*>;
extern template class Tuple_t<unsigned long long, unsigned long long*>;

extern template class Tuple_t<float, float*>;
extern template class Tuple_t<double, double*>;
extern template class Tuple_t<long double, long double*>;

/*-----------------------------------------------------------------------------
 * Static Tuple Methods
 * --------------------------------------------------------------------------*/

/*-------------------------------------
 * tuple_t<data_t...>::constructObjects (Sentinel)
 * ----------------------------------*/
template <typename... data_t>
template <typename arg_t>
constexpr
bool Tuple_t<data_t...>::construct_objs(char* buffer, unsigned offset, arg_t*) {

    return new(buffer + offset) arg_t {
    }
    != nullptr;
}

/*-------------------------------------
 * tuple_t<data_t...>::constructObjects
 * ----------------------------------*/
template <typename... data_t>
template <typename arg_t, typename... args_t>
constexpr
bool Tuple_t<data_t...>::construct_objs(char* buffer, unsigned offset, arg_t*, args_t*... args) {

    return new(buffer + offset) arg_t {
    }
    != nullptr
        ?
        construct_objs < args_t...>(buffer, offset + sizeof (arg_t), args...)
        :
        false;
}

/*-------------------------------------
 * tuple_t<data_t...>::destroyObjects
 * ----------------------------------*/
template <typename... data_t>
template <typename arg_t>
constexpr
bool Tuple_t<data_t...>::destroy_objs(char* buffer, unsigned offset, arg_t*) {
    return std::is_destructible<arg_t>()
        ?
        reinterpret_cast<arg_t*> (buffer + offset)->~arg_t(), true
        :
        throw;
}

/*-------------------------------------
 * tuple_t<data_t...>::destroyObjects
 * ----------------------------------*/
template <typename... data_t>
template <typename arg_t, typename... args_t>
inline
bool Tuple_t<data_t...>::destroy_objs(char* buffer, unsigned offset, arg_t*, args_t*... args) {
    if (std::is_destructible<arg_t>()) {
        reinterpret_cast<arg_t*> (buffer + offset)->~arg_t();
    }

    destroy_objs < args_t...>(buffer, offset + sizeof (arg_t), args...);

    return true;
}

/*-----------------------------------------------------------------------------
 * Movement Methods
 * --------------------------------------------------------------------------*/

/*-------------------------------------
 * tuple_t<data_t...>::copyObjects (Sentinel)
 * ----------------------------------*/
template <typename... data_t>
template <typename arg_t>
inline
void Tuple_t<data_t...>::copy_objs(const Tuple_t<data_t...>& agg, char* buffer, unsigned offset, arg_t*) {
    static_assert(std::is_copy_assignable<arg_t>(), "Aggregated objects must have a copy operator available.");

    arg_t* pArg = (arg_t*) (buffer + offset);

    *pArg = *agg.get_object<arg_t>();
}

/*-------------------------------------
 * tuple_t<data_t...>::copyObjects
 * ----------------------------------*/
template <typename... data_t>
template <typename arg_t, typename... args_t>
inline
void Tuple_t<data_t...>::copy_objs(const Tuple_t<data_t...>& agg, char* buffer, unsigned offset, arg_t*, args_t*... args) {
    static_assert(std::is_copy_assignable<arg_t>(), "Aggregated objects must have a copy operator available.");

    arg_t* pArg = (arg_t*) (buffer + offset);
    *pArg = *agg.get_object<arg_t>();

    copy_objs(agg, buffer, offset + sizeof (arg_t), args...);
}

/*-------------------------------------
 * tuple_t<data_t...>::moveObjects
 * ----------------------------------*/
template <typename... data_t>
template <typename arg_t>
inline
void Tuple_t<data_t...>::move_objs(Tuple_t<data_t...>&& agg, char* buffer, unsigned offset, arg_t*) {
    static_assert(std::is_copy_assignable<arg_t>(), "Aggregated objects must have a move operator available.");

    arg_t* pArg = (arg_t*) (buffer + offset);

    *pArg = std::move(*agg.get_object<arg_t>());
}

/*-------------------------------------
 * tuple_t<data_t...>::moveObjects
 * ----------------------------------*/
template <typename... data_t>
template <typename arg_t, typename... args_t>
inline
void Tuple_t<data_t...>::move_objs(Tuple_t<data_t...>&& agg, char* buffer, unsigned offset, arg_t*, args_t*... args) {
    static_assert(std::is_copy_assignable<arg_t>(), "Aggregated objects must have a move operator available.");

    arg_t* pArg = (arg_t*) (buffer + offset);
    *pArg = std::move(*agg.get_object<arg_t>());

    move_objs(std::forward < Tuple_t < data_t...>>(agg), buffer, offset + sizeof (arg_t), args...);
}

/*-----------------------------------------------------------------------------
 * Non-Static Tuple Methods
 * --------------------------------------------------------------------------*/

/*-------------------------------------
 * Private Constructor
 * ----------------------------------*/
template <typename... data_t>
constexpr Tuple_t<data_t...>::Tuple_t(bool) {
}

/*-------------------------------------
 * Constructor
 * ----------------------------------*/
template <typename... data_t>
constexpr Tuple_t<data_t...>::Tuple_t()
    :
    Tuple_t(construct_objs(dataBuffer, 0, ((data_t*) nullptr)...)) {
}

/*-------------------------------------
 * Copy Constructor
 * ----------------------------------*/
template <typename... data_t>
Tuple_t<data_t...>::Tuple_t(const Tuple_t& a)
    :
    Tuple_t{}
{
    copy_objs(a, dataBuffer, 0, ((data_t*) nullptr)...);
}

/*-------------------------------------
 * Move Constructor
 * ----------------------------------*/
template <typename... data_t>
Tuple_t<data_t...>::Tuple_t(Tuple_t&& a)
    :
    Tuple_t{}
{
    move_objs(std::forward < Tuple_t < data_t...>>(a), dataBuffer, 0, ((data_t*) nullptr)...);
}

/*-------------------------------------
 * Destructor
 * ----------------------------------*/
template <typename... data_t>
Tuple_t<data_t...>::~Tuple_t() {
    destroy_objs(dataBuffer, 0, ((data_t*) nullptr)...);
}

/*-------------------------------------
 * Copy Operator
 * ----------------------------------*/
template <typename... data_t>
Tuple_t<data_t...>& Tuple_t<data_t...>::operator=(const Tuple_t& a) {
    copy_objs(a, dataBuffer, 0, ((data_t*) nullptr)...);
    return *this;
}

/*-------------------------------------
 * Move Operator
 * ----------------------------------*/
template <typename... data_t>
Tuple_t<data_t...>& Tuple_t<data_t...>::operator=(Tuple_t&& a) {
    move_objs(std::forward < Tuple_t < data_t...>>(a), dataBuffer, 0, ((data_t*) nullptr)...);
    return *this;
}

/*-------------------------------------
 * Get an object using an its type (const)
 * ----------------------------------*/

/*-------------------------------------
 * Get the last object in the data buffer. (Sentinel)
 * ----------------------------------*/
template <typename... data_t>
template <typename request_t, typename arg_t>
constexpr
const request_t* Tuple_t<data_t...>::get_obj_at_offset(unsigned offset, arg_t*) const {
    return std::is_same<request_t, arg_t>()
        ?
        (const request_t*) (dataBuffer + offset)
        :
        nullptr;
}

/*-------------------------------------
 * Get an object from the data buffer.
 * ----------------------------------*/
template <typename... data_t>
template <typename request_t, typename arg_t, typename... args_t>
constexpr
const request_t* Tuple_t<data_t...>::get_obj_at_offset(unsigned offset, arg_t*, args_t*...) const {
    return std::is_same<request_t, arg_t>()
        ?
        (const request_t*) (dataBuffer + offset)
        :
        get_obj_at_offset<request_t, args_t...>(sizeof (arg_t) + offset, ((args_t*) nullptr)...);
}

/*-------------------------------------
 * Get an object contained within *this. This object is specified using
 * template parameters.
 * ----------------------------------*/
template <typename... data_t>
template <typename request_t>
constexpr
const request_t* Tuple_t<data_t...>::get_object() const {
    return get_obj_at_offset<request_t, data_t...>(0, ((data_t*) nullptr)...);
}

/*-------------------------------------
 * Get an object using an its type
 * ----------------------------------*/

/*-------------------------------------
 * Get the last object in the data buffer.
 * ----------------------------------*/
template <typename... data_t>
template <typename request_t, typename arg_t>
inline
request_t* Tuple_t<data_t...>::get_obj_at_offset(unsigned offset, arg_t*) {
    return std::is_same<request_t, arg_t>()
        ?
        (request_t*) (dataBuffer + offset)
        :
        nullptr;
}

/*-------------------------------------
 * Get an object from the data buffer.
 * ----------------------------------*/
template <typename... data_t>
template <typename request_t, typename arg_t, typename... args_t>
inline
request_t* Tuple_t<data_t...>::get_obj_at_offset(unsigned offset, arg_t*, args_t*...) {
    return std::is_same<request_t, arg_t>()
        ?
        (request_t*) (dataBuffer + offset)
        :
        get_obj_at_offset<request_t, args_t...>(sizeof (arg_t) + offset, ((args_t*) nullptr)...);
}

/*-------------------------------------
 * Get an object contained within *this. This object is specified using
 * template parameters.
 * ----------------------------------*/
template <typename... data_t>
template <typename request_t>
inline
request_t* Tuple_t<data_t...>::get_object() {
    return get_obj_at_offset<request_t, data_t...>(0, ((data_t*) nullptr)...);
}

/*-------------------------------------
 * Get an object using an its index (const)
 * ----------------------------------*/

/*-------------------------------------
 * Get the last object in the data buffer. (Sentinel)
 * ----------------------------------*/
template <typename... data_t>
template <typename arg_t>
constexpr
const void* Tuple_t<data_t...>::get_obj_at_index(unsigned index, unsigned offset, arg_t*) const {
    return index == 0
        ?
        (const void*) (dataBuffer + offset)
        :
        nullptr;
}

/*-------------------------------------
 * Get an object from the data buffer.
 * ----------------------------------*/
template <typename... data_t>
template <typename arg_t, typename... args_t>
constexpr
const void* Tuple_t<data_t...>::get_obj_at_index(unsigned index, unsigned offset, arg_t*, args_t*...) const {
    return (index == 0)
        ?
        (const void*) (dataBuffer + offset)
        :
        get_obj_at_index < args_t...>(index - 1, sizeof (arg_t) + offset, ((args_t*) nullptr)...);
}

/*-------------------------------------
 * Get an object contained within *this. This object is specified using
 * an array-like index.
 * ----------------------------------*/
template <typename... data_t>
constexpr
const void* Tuple_t<data_t...>::get_object(unsigned index) const {
    return get_obj_at_index < data_t...>(index, 0, ((data_t*) nullptr)...);
}

/*-------------------------------------
 * Get an object using an its index
 * ----------------------------------*/

/*-------------------------------------
 * Get the last object in the data buffer.
 * ----------------------------------*/
template <typename... data_t>
template <typename arg_t>
inline
void* Tuple_t<data_t...>::get_obj_at_index(unsigned index, unsigned offset, arg_t*) {
    return index == 0
        ?
        (void*) (dataBuffer + offset)
        :
        nullptr;
}

/*-------------------------------------
 * Get an object from the data buffer.
 * ----------------------------------*/
template <typename... data_t>
template <typename arg_t, typename... args_t>
inline
void* Tuple_t<data_t...>::get_obj_at_index(unsigned index, unsigned offset, arg_t*, args_t*...) {
    return index == 0
        ?
        (void*) (dataBuffer + offset)
        :
        get_obj_at_index < args_t...>(index - 1, sizeof (arg_t) + offset, ((args_t*) nullptr)...);
}

/*-------------------------------------
 * Get an object contained within *this. This object is specified using
 * an array-like index.
 * ----------------------------------*/
template <typename... data_t>
inline
void* Tuple_t<data_t...>::get_object(unsigned index) {
    return get_obj_at_index < data_t...>(index, 0, ((data_t*) nullptr)...);
}

/*-------------------------------------
 * Non-retrieval methods
 * ----------------------------------*/

/*-------------------------------------
 * Retrieve the number of objects contained within an tuple_t.
 * ----------------------------------*/
template <typename... data_t>
constexpr
unsigned Tuple_t<data_t...>::get_num_objects() const {
    return sizeof...(data_t);
}

} // end utils namespace
} // end ls namespace

#endif /* __LS_UTILS_TUPLE_H__ */
