
#ifndef __LS_UTILS_TUPLE_H__
#define __LS_UTILS_TUPLE_H__

#include <new>
#include <type_traits>
#include <utility>

#include "lightsky/utils/byteSize.h"

namespace ls {
namespace utils {

/*-----------------------------------------------------------------------------
    Tuple Object
-----------------------------------------------------------------------------*/
/**
 *  @brief Tuple Class
 *  An tuple is a set of objects determined at compile-time. These objects
 *  are constructed within a tightly packed buffer. It is recommended to
 *  construct an tuple using the "new" operator if there are many objects.
 */
template <typename... data_t>
class tuple_t {
    /**
     *  @brief Construct an object at a preallocated space within a buffer
     *
     *  @note
     *  The buffer must be preallocated. The pointer passed into the function
     *  is a sentinel value (for the sake of recursion) and is not used for
     *  anything.
     */
    template <typename arg_t>
    static constexpr bool constructObjects(char* buffer, unsigned offset, arg_t*);

    /**
     *  @brief Construct an object at a preallocated space within a buffer
     *
     *  @note
     *  The buffer must be preallocated. The pointers passed into the function
     *  are sentinel values (for the sake of recursion) and are not used for
     *  anything.
     */
    template <typename arg_t, typename... args_t>
    static constexpr bool constructObjects(char* buffer, unsigned offset, arg_t*, args_t*... args);
    
    /**
     *  @brief Destroy an object at a preallocated space within a buffer.
     */
    template <typename arg_t>
    static constexpr bool destroyObjects(char* buffer, unsigned offset, arg_t*);

    /**
     *  @brief Destroy an object at a preallocated space within a buffer.
     */
    template <typename arg_t, typename... args_t>
    static inline bool destroyObjects(char* buffer, unsigned offset, arg_t*, args_t*... args);
    
    /*-------------------------------------------------------------------------
        Movements and copies
    -------------------------------------------------------------------------*/
    /**
     *  @brief Copy an object at a preallocated space within a buffer
     *
     *  @note
     *  The buffer must be preallocated. The pointer passed into the function
     *  is a sentinel value (for the sake of recursion) and is not used for
     *  anything.
     */
    template <typename arg_t>
    static inline void copyObjects(const tuple_t<data_t...>&, char* buffer, unsigned offset, arg_t*);

    /**
     *  @brief Construct an object at a preallocated space within a buffer
     *
     *  @note
     *  The buffer must be preallocated. The pointers passed into the function
     *  are sentinel values (for the sake of recursion) and are not used for
     *  anything.
     */
    template <typename arg_t, typename... args_t>
    static inline void copyObjects(const tuple_t<data_t...>&, char* buffer, unsigned offset, arg_t*, args_t*... args);
    
    /**
     *  @brief Destroy an object at a preallocated space within a buffer.
     */
    template <typename arg_t>
    static inline void moveObjects(tuple_t<data_t...>&&, char* buffer, unsigned offset, arg_t*);

    /**
     *  @brief Destroy an object at a preallocated space within a buffer.
     */
    template <typename arg_t, typename... args_t>
    static inline void moveObjects(tuple_t<data_t...>&&, char* buffer, unsigned offset, arg_t*, args_t*... args);

    private:
        /**
         *  @brief Private Constructor
         *
         *  Assists in constructing an tuple at compile-time
         */
        constexpr tuple_t(bool);
        
        /**
         *  @brief dataBuffer
         *
         *  A buffer of bytes (characters) that is used to hold objects of an
         *  tuple. This allows objects to be added at compile-time and
         *  constructed in a tightly packed array.
         */
        char dataBuffer[getByteSize<data_t...>()];

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
        constexpr const request_t* getObjectAtOffset(unsigned offset, arg_t*) const;

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
        constexpr const request_t* getObjectAtOffset(unsigned offset, arg_t*, args_t*...) const;

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
        inline request_t* getObjectAtOffset(unsigned offset, arg_t*);

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
        inline request_t* getObjectAtOffset(unsigned offset, arg_t*, args_t*...);

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
        constexpr const void* getObjectAtIndex(unsigned index, unsigned offset, arg_t*) const;

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
        constexpr const void* getObjectAtIndex(unsigned index, unsigned offset, arg_t*, args_t*...) const;

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
        inline void* getObjectAtIndex(unsigned index, unsigned offset, arg_t*);

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
        inline void* getObjectAtIndex(unsigned index, unsigned offset, arg_t*, args_t*...);

    public:
        /**
         *  @brief Constructor
         *
         *  Initialize all parameterized data types within a byte array.
         */
        constexpr tuple_t();
        
        /**
         *  @brief Copy Constructor
         *  
         *  Uses each object's copy operator to copy data into *this.
         */
        tuple_t(const tuple_t&);
        
        /**
         *  @brief Move Constructor
         *  
         *  Uses each object's move operator to move data into *this.
         */
        tuple_t(tuple_t&&);

        /**
         *  @brief Destructor
         *
         *  Calls the destructor on all member objects requested at
         *  compile-time.
         */
        ~tuple_t();
        
        /**
         *  @brief Copy Operator
         *  
         *  Uses each object's copy operator to copy data into *this.
         */
        tuple_t& operator=(const tuple_t&);
        
        /**
         *  @brief Move Operator
         *  
         *  Uses each object's move operator to move data into *this.
         */
        tuple_t& operator=(tuple_t&&);
        
        /*---------------------------------------------------------------------
            Constant Methods
        ---------------------------------------------------------------------*/
        /**
         *  @brief Get a constant object contained within *this. This object is 
         *  specified using template parameters.
         *  
         *  @return
         *  A constant pointer to a tuple object if it exists. Otherwise, this
         *  methods returns NULL.
         */
        template <typename request_t>
        constexpr const request_t* getObject() const;

        /*---------------------------------------------------------------------
            Non-Constant versions of methods
        ---------------------------------------------------------------------*/
        /**
         *  @brief Get an object contained within *this. This object is
         *  specified using a template parameter.
         *  
         *  @return a pointer to a tuple object if it exists. Otherwise, this
         *  methods returns NULL.
         */
        template <typename request_t>
        inline request_t* getObject();

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
        constexpr const void* getObject(unsigned index) const;

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
        inline void* getObject(unsigned index);
        
        /**
         *  @brief Retrieve the number of objects contained within an tuple_t.
         *  
         *  @return
         *  An unsigned integral type representing the number of objects that
         *  are stored in *this.
         */
        constexpr unsigned getNumObjects() const;
};

/*-------------------------------------
    Extern Templates for plain data types.
-------------------------------------*/
extern template class tuple_t<signed char>;
extern template class tuple_t<signed char*>;
extern template class tuple_t<signed short>;
extern template class tuple_t<signed short*>;
extern template class tuple_t<signed int>;
extern template class tuple_t<signed int*>;
extern template class tuple_t<signed long>;
extern template class tuple_t<signed long*>;
extern template class tuple_t<signed long long>;
extern template class tuple_t<signed long long*>;

extern template class tuple_t<unsigned char>;
extern template class tuple_t<unsigned char*>;
extern template class tuple_t<unsigned short>;
extern template class tuple_t<unsigned short*>;
extern template class tuple_t<unsigned int>;
extern template class tuple_t<unsigned int*>;
extern template class tuple_t<unsigned long>;
extern template class tuple_t<unsigned long*>;
extern template class tuple_t<unsigned long long>;
extern template class tuple_t<unsigned long long*>;

extern template class tuple_t<float>;
extern template class tuple_t<float*>;
extern template class tuple_t<double>;
extern template class tuple_t<double*>;
extern template class tuple_t<long double>;
extern template class tuple_t<long double*>;

// data and pointer pairs
extern template class tuple_t<signed char,      signed char*>;
extern template class tuple_t<signed short,     signed short*>;
extern template class tuple_t<signed int,       signed int*>;
extern template class tuple_t<signed long,      signed long*>;
extern template class tuple_t<signed long long, signed long long*>;

extern template class tuple_t<unsigned char,        unsigned char*>;
extern template class tuple_t<unsigned short,       unsigned short*>;
extern template class tuple_t<unsigned int,         unsigned int*>;
extern template class tuple_t<unsigned long,        unsigned long*>;
extern template class tuple_t<unsigned long long,   unsigned long long*>;

extern template class tuple_t<float,        float*>;
extern template class tuple_t<double,       double*>;
extern template class tuple_t<long double,  long double*>;

} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/tuple_impl.h"

#endif /* __LS_UTILS_TUPLE_H__ */
