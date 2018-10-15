/*
 * File:   utils/Pointer.h
 * Author: miles
 *
 * Created on November 10, 2014, 10:29 PM
 */

#ifndef LS_UTILS_POINTER_H
#define LS_UTILS_POINTER_H

#include "lightsky/setup/Api.h"
#include "lightsky/setup/Macros.h"



namespace ls
{
namespace utils
{



/**----------------------------------------------------------------------------
 * Pointer Deleter Type
 * --------------------------------------------------------------------------*/
template <typename data_t>
struct PointerDeleter
{
    constexpr void operator() (void* const p) const noexcept
    {
        delete p;
    }
};



template <typename data_t>
struct PointerDeleter<data_t[]>
{
    constexpr void operator() (data_t* const p) const noexcept
    {
        delete [] p;
    }
};



/**----------------------------------------------------------------------------
 * Single Pointer Type
 * --------------------------------------------------------------------------*/
template<typename data_t, class Deleter = PointerDeleter<data_t>>
class LS_API Pointer
{

    // public typedefs
  public:
    typedef data_t value_type;

  private:
    /**
     * @brief pData represents a Pointer to some data within an application
     */
    mutable data_t* pData = nullptr;

    /**
     * @brief Clear *this of any data/resources.
     */
    inline
    void clear()
    {
        constexpr Deleter d = Deleter();
        d(pData);
    }

  public:
    /**
     * @brief Destructor
     *
     * Clear *this of any data/resources.
     */
    inline
    ~Pointer()
    {
        clear();
    }

    /**
     * @brief Constructor
     *
     * Creates an empty Pointer type. Which should not be dereferenced
     * under any circumstances.
     */
    constexpr
    Pointer() :
        pData{nullptr}
    {
    }

    /**
     * @brief Pointer Constructor
     *
     * @param p
     * A Pointer to dynamically-allocated data.
     */
    explicit
    Pointer(data_t* const p) :
        pData{p}
    {
    }

    /**
     * @brief NULL Constructor
     *
     * Constructs *this with no data assigned.
     */
    constexpr
    Pointer(std::nullptr_t) :
        pData{nullptr}
    {
    }

    /**
     * Copy Constructor -- DELETED
     */
    Pointer(const Pointer&) = delete;

    /**
     * @brief Move Constructor
     *
     * Moves data from the input parameter into *this.
     *
     * @param p
     * A Pointer type containing dynamically-allocated data.
     */
    inline
    Pointer(Pointer&& p) :
        pData{p.pData}
    {
        p.pData = nullptr;
    }

    /**
     * Copy Operator -- DELETED
     */
    Pointer& operator=(const Pointer&) = delete;

    /**
     * @brief Move Operatpr
     *
     * Moves data from the input parameter into *this.
     *
     * @param p
     * A Pointer type containing dynamically-allocated data.
     *
     * @return
     * A reference to *this.
     */
    inline
    Pointer& operator=(Pointer&& p)
    {
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
    constexpr
    bool operator!() const
    {
        return nullptr == pData;
    }

    /**
     * @brief Determine if there is any data in *this.
     *
     * @return TRUE if *this points to data, FALSE if not.
     */
    explicit constexpr
    operator bool() const
    {
        return nullptr != pData;
    }

    /**
     * @brief Equal-To Operator
     *
     * @param p
     * A constant reference to a Pointer object of the same type as *this.
     *
     * @return TRUE if the input parameter contains a Pointer to the same
     * data as *this, FALSE if not.
     */
    constexpr
    bool operator==(const Pointer& p) const
    {
        return pData == p.pData;
    }

    /**
     * @brief Not-Equal-To Operator
     *
     * @param p
     * A constant reference to a Pointer object of the same type as *this.
     *
     * @return TRUE if the input parameter contains a Pointer to different
     * data than *this, FALSE if so.
     */
    constexpr
    bool operator!=(const Pointer& p) const
    {
        return pData != p.pData;
    }

    /**
     * @brief Greater-Than or Equal-To Operator
     *
     * @param p
     * A constant reference to a Pointer object of the same type as *this.
     *
     * @return TRUE if the input parameter contains a Pointer to the same
     * data as or less than, or equal to, *this, FALSE if not.
     */
    constexpr
    bool operator>=(const Pointer& p) const
    {
        return pData >= p.pData;
    }

    /**
     * @brief Greater-Than Operator
     *
     * @param p
     * A constant reference to a Pointer object of the same type as *this.
     *
     * @return TRUE if the input parameter contains a Pointer of less
     * value than *this, FALSE if not.
     */
    constexpr
    bool operator>(const Pointer& p) const
    {
        return pData > p.pData;
    }

    /**
     * @brief Less-Than or Equal-To Operator
     *
     * @param p
     * A constant reference to a Pointer object of the same type as *this.
     *
     * @return TRUE if the input parameter contains a Pointer to the same
     * data as or greater than *this, FALSE if not.
     */
    constexpr
    bool operator<=(const Pointer& p) const
    {
        return pData <= p.pData;
    }

    /**
     * @brief Less-Than Operator
     *
     * @param p
     * A constant reference to a Pointer object of the same type as *this.
     *
     * @return TRUE if the input parameter contains a Pointer of greater
     * value than *this, FALSE if not.
     */
    constexpr
    bool operator<(const Pointer& p) const
    {
        return pData < p.pData;
    }

    /**
     * @brief Equal-To Operator
     *
     * @param p
     * A Pointer to an object of the same type as the one contained within
     * *this.
     *
     * @return TRUE if the input parameter contains a Pointer to the same
     * data as *this, FALSE if not.
     */
    constexpr
    bool operator==(const data_t* const p) const
    {
        return pData == p;
    }

    /**
     * @brief Not-Equal-To Operator
     *
     * @param p
     * A Pointer to an object of the same type as the one contained within
     * *this.
     *
     * @return TRUE if the input parameter contains a Pointer to different
     * data than *this, FALSE if so.
     */
    constexpr
    bool operator!=(const data_t* const p) const
    {
        return pData != p;
    }

    /**
     * @brief Greater-Than or Equal-To Operator
     *
     * @param p
     * A Pointer to an object of the same type as the one contained within
     * *this.
     *
     * @return TRUE if the input parameter contains a Pointer to the same
     * data as or less than, or equal to, *this, FALSE if not.
     */
    constexpr
    bool operator>=(const data_t* const p) const
    {
        return pData >= p;
    }

    /**
     * @brief Greater-Than Operator
     *
     * @param p
     * A Pointer to an object of the same type as the one contained within
     * *this.
     *
     * @return TRUE if the input parameter contains a Pointer of less
     * value than *this, FALSE if not.
     */
    constexpr
    bool operator>(const data_t* const p) const
    {
        return pData > p;
    }

    /**
     * @brief Less-Than or Equal-To Operator
     *
     * @param p
     * A Pointer to an object of the same type as the one contained within
     * *this.
     *
     * @return TRUE if the input parameter contains a Pointer to the same
     * data as or greater than *this, FALSE if not.
     */
    constexpr
    bool operator<=(const data_t* const p) const
    {
        return pData <= p;
    }

    /**
     * @brief Less-Than Operator
     *
     * @param p
     * A Pointer to an object of the same type as the one contained within
     * *this.
     *
     * @return TRUE if the input parameter contains a Pointer of greater
     * value than *this, FALSE if not.
     */
    constexpr
    bool operator<(const data_t* const p) const
    {
        return pData < p;
    }

    /**
     * Retrieve the Pointer to data contained within *this.
     *
     * @return A Pointer to a set of dynamically-allocated data.
     */
    constexpr
    const data_t* get() const
    {
        return pData;
    }

    /**
     * @brief Retrieve the Pointer to data contained within *this.
     *
     * @return A Pointer to a set of dynamically-allocated data.
     */
    inline
    data_t* get()
    {
        return pData;
    }

    /**
     * @brief Swap the value of the Pointers contained within *this and an
     * input Pointer object.
     *
     * @param other
     * A Pointer object who's data should be swapped with *this.
     */
    inline
    void swap(Pointer& other)
    {
        data_t* temp = other.pData;
        other.pData = this->pData;
        this->pData = temp;
    }

    /**
     * @brief Retrieve a reference to the data contained within *this.
     *
     * @return A reference to the dynamically-allocated data within *this.
     */
    inline
    const data_t& operator*() const
    {
        return *pData;
    }

    /**
     * @brief Retrieve a reference to the data contained within *this.
     *
     * @return A reference to the dynamically-allocated data within *this.
     */
    inline
    data_t& operator*()
    {
        return *pData;
    }

    /**
     * @brief Retrieve a constant member contained within the data pointed
     * at by *this.
     *
     * @return A constant Pointer to a member of the dynamically-allocated
     * data within *this.
     */
    inline
    const data_t* operator->() const
    {
        return pData;
    }

    /**
     * @brief Retrieve a member contained within the data pointed at by *this.
     *
     * @return A Pointer to a member of the dynamically-allocated data
     * within *this.
     */
    inline
    data_t* operator->()
    {
        return pData;
    }

    /**
     * @brief explicitly cast *this to the original Pointer type contained
     * within *this.
     *
     * @return A Pointer to a constant object Pointer such as the one
     * contained within *this.
     */
    constexpr
    operator const data_t*() const
    {
        return pData;
    }

    /**
     * @brief explicitly cast *this to the original Pointer type contained
     * within *this.
     *
     * @return A Pointer to an object Pointer such as the one contained
     * within *this.
     */
    inline
    operator data_t*()
    {
        return pData;
    }

    /**
     * @brief Delete the value of the internal Pointer managed by this,
     * then use the input parameter to reference a new set of data.
     *
     * @param pNewData
     * A Pointer to a set of dynamically-allocated memory of the same type
     * as *this.
     */
    inline
    void reset(data_t* pNewData = nullptr)
    {
        clear();
        pData = pNewData;
    }

    /**
     * @brief Return the currently held pointer to data after releasing
     * all ownership.
     *
     * This method will release ownership of all data that *this object
     * references.
     *
     * @return A pointer to the currently held data store after it has been
     * relieved of all internal references.
     */
    inline
    data_t* release()
    {
        data_t* const pRet = pData;
        pData = nullptr;
        return pRet;
    }
};



/**----------------------------------------------------------------------------
 * Array Pointer Type
 * (Specialized in order to allow for array-types)
 * --------------------------------------------------------------------------*/
template<typename data_t, class Deleter>
class LS_API Pointer<data_t[], Deleter>
{

    // public typedefs
  public:
    typedef data_t value_type;

  private:
    /**
     * pData represents a Pointer to some data within an application
     */
    mutable data_t* pData = nullptr;

    /**
     * Clear *this of any data/resources.
     */
    inline
    void clear()
    {
        constexpr Deleter d = Deleter();
        d(pData);
    }

  public:

    /**
     * @brief Destructor
     *
     * Clear *this of any data/resources.
     */
    inline
    ~Pointer()
    {
        clear();
    }

    /**
     * @brief Constructor
     *
     * Creates an empty Pointer type. Which should not be dereferenced
     * under any circumstances.
     */
    constexpr
    Pointer() :
        pData{nullptr}
    {
    }

    /**
     * @brief Pointer Constructor
     *
     * @param p
     * A Pointer to dynamically-allocated data.
     */
    explicit
    Pointer(data_t* const p) :
        pData{p}
    {
    }

    /**
     * @brief NULL Constructor
     *
     * Constructs *this with no data assigned.
     */
    constexpr
    Pointer(std::nullptr_t) :
        pData{nullptr}
    {
    }

    /**
     * Copy Constructor -- DELETED
     */
    Pointer(const Pointer&) = delete;

    /**
     * @brief Move Constructor
     *
     * Moves data from the input parameter into *this.
     *
     * @param p
     * A Pointer type containing dynamically-allocated data.
     */
    inline
    Pointer(Pointer&& p) :
        pData{p.pData}
    {
        p.pData = nullptr;
    }

    /**
     * Copy Operator -- DELETED
     */
    Pointer& operator=(const Pointer&) = delete;

    /**
     * @brief Move Operatpr
     *
     * Moves data from the input parameter into *this.
     *
     * @param p
     * A Pointer type containing dynamically-allocated data.
     *
     * @return
     * A reference to *this.
     */
    inline
    Pointer& operator=(Pointer&& p)
    {
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
    constexpr
    bool operator!() const
    {
        return nullptr == pData;
    }

    /**
     * @brief Determine if there is any data in *this.
     *
     * @return TRUE if *this points to data, FALSE if not.
     */
    explicit constexpr
    operator bool() const
    {
        return nullptr != pData;
    }

    /**
     * @brief Equal-To Operator
     *
     * @param p
     * A constant reference to a Pointer object of the same type as *this.
     *
     * @return TRUE if the input parameter contains a Pointer to the same
     * data as *this, FALSE if not.
     */
    constexpr
    bool operator==(const Pointer& p) const
    {
        return pData == p.pData;
    }

    /**
     * @brief Not-Equal-To Operator
     *
     * @param p
     * A constant reference to a Pointer object of the same type as *this.
     *
     * @return TRUE if the input parameter contains a Pointer to different
     * data than *this, FALSE if so.
     */
    constexpr
    bool operator!=(const Pointer& p) const
    {
        return pData != p.pData;
    }

    /**
     * @brief Greater-Than or Equal-To Operator
     *
     * @param p
     * A constant reference to a Pointer object of the same type as *this.
     *
     * @return TRUE if the input parameter contains a Pointer to the same
     * data as or less than, or equal to, *this, FALSE if not.
     */
    constexpr
    bool operator>=(const Pointer& p) const
    {
        return pData >= p.pData;
    }

    /**
     * @brief Greater-Than Operator
     *
     * @param p
     * A constant reference to a Pointer object of the same type as *this.
     *
     * @return TRUE if the input parameter contains a Pointer of less
     * value than *this, FALSE if not.
     */
    constexpr
    bool operator>(const Pointer& p) const
    {
        return pData > p.pData;
    }

    /**
     * @brief Less-Than or Equal-To Operator
     *
     * @param p
     * A constant reference to a Pointer object of the same type as *this.
     *
     * @return TRUE if the input parameter contains a Pointer to the same
     * data as or greater than *this, FALSE if not.
     */
    constexpr
    bool operator<=(const Pointer& p) const
    {
        return pData <= p.pData;
    }

    /**
     * @brief Less-Than Operator
     *
     * @param p
     * A constant reference to a Pointer object of the same type as *this.
     *
     * @return TRUE if the input parameter contains a Pointer of greater
     * value than *this, FALSE if not.
     */
    constexpr
    bool operator<(const Pointer& p) const
    {
        return pData < p.pData;
    }

    /**
     * @brief Equal-To Operator
     *
     * @param p
     * A Pointer to an object of the same type as the one contained within
     * *this.
     *
     * @return TRUE if the input parameter contains a Pointer to the same
     * data as *this, FALSE if not.
     */
    constexpr
    bool operator==(const data_t* const p) const
    {
        return pData == p;
    }

    /**
     * @brief Not-Equal-To Operator
     *
     * @param p
     * A Pointer to an object of the same type as the one contained within
     * *this.
     *
     * @return TRUE if the input parameter contains a Pointer to different
     * data than *this, FALSE if so.
     */
    constexpr
    bool operator!=(const data_t* const p) const
    {
        return pData != p;
    }

    /**
     * @brief Greater-Than or Equal-To Operator
     *
     * @param p
     * A Pointer to an object of the same type as the one contained within
     * *this.
     *
     * @return TRUE if the input parameter contains a Pointer to the same
     * data as or less than, or equal to, *this, FALSE if not.
     */
    constexpr
    bool operator>=(const data_t* const p) const
    {
        return pData >= p;
    }

    /**
     * @brief Greater-Than Operator
     *
     * @param p
     * A Pointer to an object of the same type as the one contained within
     * *this.
     *
     * @return TRUE if the input parameter contains a Pointer of less
     * value than *this, FALSE if not.
     */
    constexpr
    bool operator>(const data_t* const p) const
    {
        return pData > p;
    }

    /**
     * @brief Less-Than or Equal-To Operator
     *
     * @param p
     * A Pointer to an object of the same type as the one contained within
     * *this.
     *
     * @return TRUE if the input parameter contains a Pointer to the same
     * data as or greater than *this, FALSE if not.
     */
    constexpr
    bool operator<=(const data_t* const p) const
    {
        return pData <= p;
    }

    /**
     * @brief Less-Than Operator
     *
     * @param p
     * A Pointer to an object of the same type as the one contained within
     * *this.
     *
     * @return TRUE if the input parameter contains a Pointer of greater
     * value than *this, FALSE if not.
     */
    constexpr
    bool operator<(const data_t* const p) const
    {
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
    constexpr
    data_t& operator[](const int i) const
    {
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
    inline
    data_t& operator[](const int i)
    {
        return pData[i];
    }

    /**
     * @brief Retrieve the Pointer to data contained within *this.
     *
     * @return A Pointer to a set of dynamically-allocated data.
     */
    constexpr
    const data_t* get() const
    {
        return pData;
    }

    /**
     * @brief Retrieve the Pointer to data contained within *this.
     *
     * @return A Pointer to a set of dynamically-allocated data.
     */
    inline
    data_t* get()
    {
        return pData;
    }

    /**
     * @brief Swap the value of the Pointers contained within *this and an
     * input Pointer object.
     *
     * @param other
     * A Pointer object who's data should be swapped with *this.
     */
    inline
    void swap(Pointer& other)
    {
        data_t* temp = other.pData;
        other.pData = this->pData;
        this->pData = temp;
    }

    /**
     * @brief Retrieve a reference to the data contained within *this.
     *
     * @return A reference to the dynamically-allocated data within *this.
     */
    inline
    const data_t& operator*() const
    {
        return *pData;
    }

    /**
     * @brief Retrieve a reference to the data contained within *this.
     *
     * @return A reference to the dynamically-allocated data within *this.
     */
    inline
    data_t& operator*()
    {
        return *pData;
    }

    /**
     * @brief Retrieve a constant member contained within the data pointed
     * at by *this.
     *
     * @return A constant Pointer to a member of the dynamically-allocated
     * data within *this.
     */
    inline
    const data_t* operator->() const
    {
        return pData;
    }

    /**
     * @brief Retrieve a member contained within the data pointed at by *this.
     *
     * @return A Pointer to a member of the dynamically-allocated data
     * within *this.
     */
    inline
    data_t* operator->()
    {
        return pData;
    }

    /**
     * @brief explicitly cast *this to the original Pointer type contained
     * within *this.
     *
     * @return A Pointer to a constant object Pointer such as the one
     * contained within *this.
     */
    constexpr
    operator const data_t*() const
    {
        return pData;
    }

    /**
     * @brief explicitly cast *this to the original Pointer type contained
     * within *this.
     *
     * @return A Pointer to an object Pointer such as the one contained
     * within *this.
     */
    inline
    operator data_t*()
    {
        return pData;
    }

    /**
     * @brief Delete the value of the internal Pointer managed by this,
     * then use the input parameter to reference a new set of data.
     *
     * @param pNewData
     * A Pointer to a set of dynamically-allocated memory of the same type
     * as *this.
     */
    inline
    void reset(data_t* pNewData = nullptr)
    {
        clear();
        pData = pNewData;
    }

    /**
     * @brief Return the currently held pointer to data after releasing
     * all ownership.
     *
     * This method will release ownership of all data that *this object
     * references.
     *
     * @return A pointer to the currently held data store after it has been
     * relieved of all internal references.
     */
    inline
    data_t* release()
    {
        data_t* const pRet = pData;
        pData = nullptr;
        return pRet;
    }
};



} // end utils namespace
} // end ls namespace



#endif  /* LS_UTILS_POINTER_H */
