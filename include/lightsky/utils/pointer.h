/* 
 * File:   utils/pointer.h
 * Author: miles
 *
 * Created on November 10, 2014, 10:29 PM
 */

#ifndef __LS_UTILS_POINTER_H__
#define	__LS_UTILS_POINTER_H__

#include "lightsky/setup/macros.h"

namespace ls {
namespace utils {

/**----------------------------------------------------------------------------
 * Single Pointer Type
-----------------------------------------------------------------------------*/
template <typename data_t>
class pointer {
    private:
        /**
         * @brief pData represents a pointer to some data within an application
         */
        mutable data_t* pData = nullptr;
        
        /**
         * @brief Clear *this of any data/resources.
         */
        void clear() {
            delete pData;
        }
        
    public:
        /**
         * @brief Destructor
         * 
         * Clear *this of any data/resources.
         */
        ~pointer() {
            clear();
        }
        
        /**
         * @brief Constructor
         * 
         * Creates an empty pointer type. Which should not be dereferenced
         * under any circumstances.
         */
        constexpr pointer() :
            pData{nullptr}
        {}
        
        /**
         * @brief Pointer Constructor
         * 
         * @param p
         * A pointer to dynamically-allocated data.
         */
        explicit pointer(data_t* const p) :
            pData{p}
        {}
        
        /**
         * @brief NULL Constructor
         * 
         * Constructs *this with no data assigned.
         */
        constexpr pointer(std::nullptr_t) :
            pData{nullptr}
        {}
        
        /**
         * Copy Constructor -- DE:ETED
         */
        pointer(const pointer&) = delete;
        
        /**
         * @brief Move Constructor
         * 
         * Moves data from the input parameter into *this.
         * 
         * @param p
         * A pointer type containing dynamically-allocated data.
         */
        pointer(pointer&& p) :
            pData{p.pData}
        {
            p.pData = nullptr;
        }
        
        /**
         * Copy Operator -- DELETED
         */
        pointer& operator=(const pointer&) = delete;
        
        /**
         * @brief Move Operatpr
         * 
         * Moves data from the input parameter into *this.
         * 
         * @param p
         * A pointer type containing dynamically-allocated data.
         * 
         * @return
         * A reference to *this.
         */
        pointer& operator=(pointer&& p) {
            clear();
            pData = p.pData;
            p.pData = nullptr;
            return *this;
        }
        
        /**
         * @brief Logical 'not' operator (negation).
         * 
         * @return TRUE if *this object points to any data, FALSE if not.
         */
        constexpr bool operator ! () const {
            return nullptr == pData;
        }
        
        /**
         * @brief Determine if there is any data in *this.
         * 
         * @return TRUE if *this points to data, FALSE if not.
         */
        explicit constexpr operator bool() const {
            return nullptr != pData;
        } 
        
        /**
         * @brief Equal-To Operator
         * 
         * @param p
         * A constant reference to a pointer object of the same type as *this.
         * 
         * @return TRUE if the input parameter contains a pointer to the same
         * data as *this, FALSE if not.
         */
        constexpr bool operator == (const pointer& p) const {
            return pData == p.pData;
        }
        
        /**
         * @brief Not-Equal-To Operator
         * 
         * @param p
         * A constant reference to a pointer object of the same type as *this.
         * 
         * @return TRUE if the input parameter contains a pointer to different
         * data than *this, FALSE if so.
         */
        constexpr bool operator != (const pointer& p) const {
            return pData != p.pData;
        }
        
        /**
         * @brief Greater-Than or Equal-To Operator
         * 
         * @param p
         * A constant reference to a pointer object of the same type as *this.
         * 
         * @return TRUE if the input parameter contains a pointer to the same
         * data as or less than, or equal to, *this, FALSE if not.
         */
        constexpr bool operator >= (const pointer& p) const {
            return pData >= p.pData;
        }
        
        /**
         * @brief Greater-Than Operator
         * 
         * @param p
         * A constant reference to a pointer object of the same type as *this.
         * 
         * @return TRUE if the input parameter contains a pointer of less
         * value than *this, FALSE if not.
         */
        constexpr bool operator > (const pointer& p) const {
            return pData > p.pData;
        }
        
        /**
         * @brief Less-Than or Equal-To Operator
         * 
         * @param p
         * A constant reference to a pointer object of the same type as *this.
         * 
         * @return TRUE if the input parameter contains a pointer to the same
         * data as or greater than *this, FALSE if not.
         */
        constexpr bool operator <= (const pointer& p) const {
            return pData <= p.pData;
        }
        
        /**
         * @brief Less-Than Operator
         * 
         * @param p
         * A constant reference to a pointer object of the same type as *this.
         * 
         * @return TRUE if the input parameter contains a pointer of greater
         * value than *this, FALSE if not.
         */
        constexpr bool operator < (const pointer& p) const {
            return pData < p.pData;
        }
        
        /**
         * @brief Equal-To Operator
         * 
         * @param p
         * A pointer to an object of the same type as the one contained within
         * *this.
         * 
         * @return TRUE if the input parameter contains a pointer to the same
         * data as *this, FALSE if not.
         */
        constexpr bool operator == (const data_t* const p) const {
            return pData == p;
        }
        
        /**
         * @brief Not-Equal-To Operator
         * 
         * @param p
         * A pointer to an object of the same type as the one contained within
         * *this.
         * 
         * @return TRUE if the input parameter contains a pointer to different
         * data than *this, FALSE if so.
         */
        constexpr bool operator != (const data_t* const p) const {
            return pData != p;
        }
        
        /**
         * @brief Greater-Than or Equal-To Operator
         * 
         * @param p
         * A pointer to an object of the same type as the one contained within
         * *this.
         * 
         * @return TRUE if the input parameter contains a pointer to the same
         * data as or less than, or equal to, *this, FALSE if not.
         */
        constexpr bool operator >= (const data_t* const p) const {
            return pData >= p;
        }
        
        /**
         * @brief Greater-Than Operator
         * 
         * @param p
         * A pointer to an object of the same type as the one contained within
         * *this.
         * 
         * @return TRUE if the input parameter contains a pointer of less
         * value than *this, FALSE if not.
         */
        constexpr bool operator > (const data_t* const p) const {
            return pData > p;
        }
        
        /**
         * @brief Less-Than or Equal-To Operator
         * 
         * @param p
         * A pointer to an object of the same type as the one contained within
         * *this.
         * 
         * @return TRUE if the input parameter contains a pointer to the same
         * data as or greater than *this, FALSE if not.
         */
        constexpr bool operator <= (const data_t* const p) const {
            return pData <= p;
        }
        
        /**
         * @brief Less-Than Operator
         * 
         * @param p
         * A pointer to an object of the same type as the one contained within
         * *this.
         * 
         * @return TRUE if the input parameter contains a pointer of greater
         * value than *this, FALSE if not.
         */
        constexpr bool operator < (const data_t* const p) const {
            return pData < p;
        }
        
        /**
         * Retrieve the pointer to data contained within *this.
         * 
         * @return A pointer to a set of dynamically-allocated data.
         */
        constexpr const data_t* get() const {
            return pData;
        }
        
        /**
         * @brief Retrieve the pointer to data contained within *this.
         * 
         * @return A pointer to a set of dynamically-allocated data.
         */
        inline data_t* get() {
            return pData;
        }
        
        /**
         * @brief Swap the value of the pointers contained within *this and an
         * input pointer object.
         * 
         * @param other
         * A pointer object who's data should be swapped with *this.
         */
        void swap(pointer& other) {
            data_t* temp = other.pData;
            other.pData = this->pData;
            this->pData = temp;
        }
        
        /**
         * @brief Retrieve a reference to the data contained within *this.
         * 
         * @return A reference to the dynamically-allocated data within *this.
         */
        const data_t& operator *() const {
            return *pData;
        }
        
        /**
         * @brief Retrieve a reference to the data contained within *this.
         * 
         * @return A reference to the dynamically-allocated data within *this.
         */
        inline data_t& operator *() {
            return *pData;
        }
        
        /**
         * @brief Retrieve a constant member contained within the data pointed
         * at by *this.
         * 
         * @return A constant pointer to a member of the dynamically-allocated
         * data within *this.
         */
        const data_t* operator ->() const {
            return pData;
        }
        
        /**
         * @brief Retrieve a member contained within the data pointed at by *this.
         * 
         * @return A pointer to a member of the dynamically-allocated data
         * within *this.
         */
        data_t* operator ->() {
            return pData;
        }
        
        /**
         * @brief explicitly cast *this to the original pointer type contained
         * within *this.
         * 
         * @return A pointer to a constant object pointer such as the one
         * contained within *this.
         */
        constexpr operator const data_t*() const {
            return pData;
        }
        
        /**
         * @brief explicitly cast *this to the original pointer type contained
         * within *this.
         * 
         * @return A pointer to an object pointer such as the one contained
         * within *this.
         */
        inline operator data_t*() {
            return pData;
        }
        
        /**
         * @brief Delete the value of the internal pointer managed by this,
         * then use *this to reference a new set of data.
         * 
         * @param pNewData
         * A pointer to a set of dynamically-allocated memory of the same type
         * as *this.
         */
        void reset(data_t* pNewData = nullptr) {
            clear();
            pData = pNewData;
        }
        
        /**
         * @brief Free all data referenced by *this.
         * 
         * This method will delete all data that *this object references. It is
         * exactly the same as calling "reset(nullptr)".
         */
        inline void release() {
            reset(nullptr);
        }
};

/*-----------------------------------------------------------------------------
 * Dynamic Pointer Types
-----------------------------------------------------------------------------*/
LS_DECLARE_CLASS_TYPE(bool_pointer, pointer, bool);
LS_DECLARE_CLASS_TYPE(char_pointer, pointer, signed char);
LS_DECLARE_CLASS_TYPE(uchar_pointer, pointer, unsigned char);
LS_DECLARE_CLASS_TYPE(wchar_pointer, pointer, wchar_t);
LS_DECLARE_CLASS_TYPE(char16_pointer, pointer, char16_t);
LS_DECLARE_CLASS_TYPE(char32_pointer, pointer, char32_t);
LS_DECLARE_CLASS_TYPE(short_pointer, pointer, signed short);
LS_DECLARE_CLASS_TYPE(ushort_pointer, pointer, unsigned short);
LS_DECLARE_CLASS_TYPE(int_pointer, pointer, signed int);
LS_DECLARE_CLASS_TYPE(uint_pointer, pointer, unsigned int);
LS_DECLARE_CLASS_TYPE(long_pointer, pointer, signed long);
LS_DECLARE_CLASS_TYPE(ulong_pointer, pointer, unsigned long);
LS_DECLARE_CLASS_TYPE(llong_pointer, pointer, signed long long);
LS_DECLARE_CLASS_TYPE(ullong_pointer, pointer, unsigned long long);
LS_DECLARE_CLASS_TYPE(float_pointer, pointer, float);
LS_DECLARE_CLASS_TYPE(double_pointer, pointer, double);
LS_DECLARE_CLASS_TYPE(ldouble_pointer, pointer, long double);

/**----------------------------------------------------------------------------
 * Array Pointer Type
 * (Specialized in order to allow for array-types)
-----------------------------------------------------------------------------*/
//template <>
template <typename data_t>
class pointer<data_t[]> {
    private:
        /**
         * pData represents a pointer to some data within an application
         */
        mutable data_t* pData = nullptr;
        
        /**
         * Clear *this of any data/resources.
         */
        void clear() {
            delete [] pData;
        }
        
    public:
        /**
         * @brief Destructor
         * 
         * Clear *this of any data/resources.
         */
        ~pointer() {
            clear();
        }
        
        /**
         * @brief Constructor
         * 
         * Creates an empty pointer type. Which should not be dereferenced
         * under any circumstances.
         */
        constexpr pointer() :
            pData{nullptr}
        {}
        
        /**
         * @brief Pointer Constructor
         * 
         * @param p
         * A pointer to dynamically-allocated data.
         */
        explicit pointer(data_t* const p) :
            pData{p}
        {}
        
        /**
         * @brief NULL Constructor
         * 
         * Constructs *this with no data assigned.
         */
        constexpr pointer(std::nullptr_t) :
            pData{nullptr}
        {}
        
        /**
         * Copy Constructor -- DE:ETED
         */
        pointer(const pointer&) = delete;
        
        /**
         * @brief Move Constructor
         * 
         * Moves data from the input parameter into *this.
         * 
         * @param p
         * A pointer type containing dynamically-allocated data.
         */
        pointer(pointer&& p) :
            pData{p.pData}
        {
            p.pData = nullptr;
        }
        
        /**
         * Copy Operator -- DELETED
         */
        pointer& operator=(const pointer&) = delete;
        
        /**
         * @brief Move Operatpr
         * 
         * Moves data from the input parameter into *this.
         * 
         * @param p
         * A pointer type containing dynamically-allocated data.
         * 
         * @return
         * A reference to *this.
         */
        pointer& operator=(pointer&& p) {
            clear();
            pData = p.pData;
            p.pData = nullptr;
            return *this;
        }
        
        /**
         * @brief Logical 'not' operator (negation).
         * 
         * @return TRUE if *this object points to any data, FALSE if not.
         */
        constexpr bool operator ! () const {
            return nullptr == pData;
        }
        
        /**
         * @brief Determine if there is any data in *this.
         * 
         * @return TRUE if *this points to data, FALSE if not.
         */
        explicit constexpr operator bool() const {
            return nullptr != pData;
        } 
        
        /**
         * @brief Equal-To Operator
         * 
         * @param p
         * A constant reference to a pointer object of the same type as *this.
         * 
         * @return TRUE if the input parameter contains a pointer to the same
         * data as *this, FALSE if not.
         */
        constexpr bool operator == (const pointer& p) const {
            return pData == p.pData;
        }
        
        /**
         * @brief Not-Equal-To Operator
         * 
         * @param p
         * A constant reference to a pointer object of the same type as *this.
         * 
         * @return TRUE if the input parameter contains a pointer to different
         * data than *this, FALSE if so.
         */
        constexpr bool operator != (const pointer& p) const {
            return pData != p.pData;
        }
        
        /**
         * @brief Greater-Than or Equal-To Operator
         * 
         * @param p
         * A constant reference to a pointer object of the same type as *this.
         * 
         * @return TRUE if the input parameter contains a pointer to the same
         * data as or less than, or equal to, *this, FALSE if not.
         */
        constexpr bool operator >= (const pointer& p) const {
            return pData >= p.pData;
        }
        
        /**
         * @brief Greater-Than Operator
         * 
         * @param p
         * A constant reference to a pointer object of the same type as *this.
         * 
         * @return TRUE if the input parameter contains a pointer of less
         * value than *this, FALSE if not.
         */
        constexpr bool operator > (const pointer& p) const {
            return pData > p.pData;
        }
        
        /**
         * @brief Less-Than or Equal-To Operator
         * 
         * @param p
         * A constant reference to a pointer object of the same type as *this.
         * 
         * @return TRUE if the input parameter contains a pointer to the same
         * data as or greater than *this, FALSE if not.
         */
        constexpr bool operator <= (const pointer& p) const {
            return pData <= p.pData;
        }
        
        /**
         * @brief Less-Than Operator
         * 
         * @param p
         * A constant reference to a pointer object of the same type as *this.
         * 
         * @return TRUE if the input parameter contains a pointer of greater
         * value than *this, FALSE if not.
         */
        constexpr bool operator < (const pointer& p) const {
            return pData < p.pData;
        }
        
        /**
         * @brief Equal-To Operator
         * 
         * @param p
         * A pointer to an object of the same type as the one contained within
         * *this.
         * 
         * @return TRUE if the input parameter contains a pointer to the same
         * data as *this, FALSE if not.
         */
        constexpr bool operator == (const data_t* const p) const {
            return pData == p;
        }
        
        /**
         * @brief Not-Equal-To Operator
         * 
         * @param p
         * A pointer to an object of the same type as the one contained within
         * *this.
         * 
         * @return TRUE if the input parameter contains a pointer to different
         * data than *this, FALSE if so.
         */
        constexpr bool operator != (const data_t* const p) const {
            return pData != p;
        }
        
        /**
         * @brief Greater-Than or Equal-To Operator
         * 
         * @param p
         * A pointer to an object of the same type as the one contained within
         * *this.
         * 
         * @return TRUE if the input parameter contains a pointer to the same
         * data as or less than, or equal to, *this, FALSE if not.
         */
        constexpr bool operator >= (const data_t* const p) const {
            return pData >= p;
        }
        
        /**
         * @brief Greater-Than Operator
         * 
         * @param p
         * A pointer to an object of the same type as the one contained within
         * *this.
         * 
         * @return TRUE if the input parameter contains a pointer of less
         * value than *this, FALSE if not.
         */
        constexpr bool operator > (const data_t* const p) const {
            return pData > p;
        }
        
        /**
         * @brief Less-Than or Equal-To Operator
         * 
         * @param p
         * A pointer to an object of the same type as the one contained within
         * *this.
         * 
         * @return TRUE if the input parameter contains a pointer to the same
         * data as or greater than *this, FALSE if not.
         */
        constexpr bool operator <= (const data_t* const p) const {
            return pData <= p;
        }
        
        /**
         * @brief Less-Than Operator
         * 
         * @param p
         * A pointer to an object of the same type as the one contained within
         * *this.
         * 
         * @return TRUE if the input parameter contains a pointer of greater
         * value than *this, FALSE if not.
         */
        constexpr bool operator < (const data_t* const p) const {
            return pData < p;
        }
        
        /**
         * @brief Iterate through all of the memory managed by *this.
         * 
         * @param i
         * The index of the data in the internal array managed by *this.
         * 
         * @return
         * A constant reference to some data contained within the
         * heap-allocated array managed by *this.
         */
        constexpr data_t& operator [] (std::size_t i) const {
            return pData[i];
        }
        
        /**
         * @brief Iterate through all of the memory managed by *this.
         * 
         * @param i
         * The index of the data in the internal array managed by *this.
         * 
         * @return
         * A reference to some data contained within the heap-allocated array
         * managed by *this.
         */
        inline data_t& operator [] (std::size_t i) {
            return pData[i];
        }
        
        /**
         * @brief Retrieve the pointer to data contained within *this.
         * 
         * @return A pointer to a set of dynamically-allocated data.
         */
        constexpr const data_t* get() const {
            return pData;
        }
        
        /**
         * @brief Retrieve the pointer to data contained within *this.
         * 
         * @return A pointer to a set of dynamically-allocated data.
         */
        inline data_t* get() {
            return pData;
        }
        
        /**
         * @brief Swap the value of the pointers contained within *this and an
         * input pointer object.
         * 
         * @param other
         * A pointer object who's data should be swapped with *this.
         */
        void swap(pointer& other) {
            data_t* temp = other.pData;
            other.pData = this->pData;
            this->pData = temp;
        }
        
        /**
         * @brief Retrieve a reference to the data contained within *this.
         * 
         * @return A reference to the dynamically-allocated data within *this.
         */
        const data_t& operator *() const {
            return *pData;
        }
        
        /**
         * @brief Retrieve a reference to the data contained within *this.
         * 
         * @return A reference to the dynamically-allocated data within *this.
         */
        data_t& operator *() {
            return *pData;
        }
        
        /**
         * @brief Retrieve a constant member contained within the data pointed
         * at by *this.
         * 
         * @return A constant pointer to a member of the dynamically-allocated
         * data within *this.
         */
        const data_t* operator ->() const {
            return pData;
        }
        
        /**
         * @brief Retrieve a member contained within the data pointed at by *this.
         * 
         * @return A pointer to a member of the dynamically-allocated data
         * within *this.
         */
        data_t* operator ->() {
            return pData;
        }
        
        /**
         * @brief explicitly cast *this to the original pointer type contained
         * within *this.
         * 
         * @return A pointer to a constant object pointer such as the one
         * contained within *this.
         */
        constexpr operator const data_t*() const {
            return pData;
        }
        
        /**
         * @brief explicitly cast *this to the original pointer type contained
         * within *this.
         * 
         * @return A pointer to an object pointer such as the one contained
         * within *this.
         */
        inline operator data_t*() {
            return pData;
        }
        
        /**
         * @brief Delete the value of the internal pointer managed by this,
         * then use *this to reference a new set of data.
         * 
         * @param pNewData
         * A pointer to a set of dynamically-allocated memory of the same type
         * as *this.
         */
        void reset(data_t* pNewData = nullptr) {
            clear();
            pData = pNewData;
        }
        
        /**
         * @brief Free all data referenced by *this.
         * 
         * This method will delete all data that *this object references. It is
         * exactly the same as calling "reset(nullptr)".
         */
        inline void release() {
            reset(nullptr);
        }
};

/*-----------------------------------------------------------------------------
 * Dynamic Array Types
-----------------------------------------------------------------------------*/
LS_DECLARE_CLASS_TYPE(bool_array, pointer, bool[]);
LS_DECLARE_CLASS_TYPE(char_array, pointer, signed char[]);
LS_DECLARE_CLASS_TYPE(uchar_array, pointer, unsigned char[]);
LS_DECLARE_CLASS_TYPE(wchar_array, pointer, wchar_t[]);
LS_DECLARE_CLASS_TYPE(char16_array, pointer, char16_t[]);
LS_DECLARE_CLASS_TYPE(char32_array, pointer, char32_t[]);
LS_DECLARE_CLASS_TYPE(short_array, pointer, signed short[]);
LS_DECLARE_CLASS_TYPE(ushort_array, pointer, unsigned short[]);
LS_DECLARE_CLASS_TYPE(int_array, pointer, signed int[]);
LS_DECLARE_CLASS_TYPE(uint_array, pointer, unsigned int[]);
LS_DECLARE_CLASS_TYPE(long_array, pointer, signed long[]);
LS_DECLARE_CLASS_TYPE(ulong_array, pointer, unsigned long[]);
LS_DECLARE_CLASS_TYPE(llong_array, pointer, signed long long[]);
LS_DECLARE_CLASS_TYPE(ullong_array, pointer, unsigned long long[]);
LS_DECLARE_CLASS_TYPE(float_array, pointer, float[]);
LS_DECLARE_CLASS_TYPE(double_array, pointer, double[]);
LS_DECLARE_CLASS_TYPE(ldouble_array, pointer, long double[]);

} // end utils namespace
} // end ls namespace

#endif	/* __LS_UTILS_POINTER_H__ */
