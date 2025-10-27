/*
 * File:   Function.hpp
 * Author: miles
 * Created on October 27, 2025, at 9:43 a.m.
 */

#ifndef LS_UTILS_FUNCTION_HPP
#define LS_UTILS_FUNCTION_HPP

#include "lightsky/utils/Copy.h" // fast_memcpy(), fast_memset()

namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
template <typename Empty>
class Function;

template <typename ResultType, typename... ArgsType>
class Function<ResultType(ArgsType...)>;

template <typename ResultType>
class Function<ResultType()>;



/*-----------------------------------------------------------------------------
 * Function Metadata
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Metadata with arguments
-------------------------------------*/
template <typename T>
class FunctionBase;

template <typename ResultType, typename... ArgsType>
class FunctionBase<ResultType(ArgsType...)>
{
public:
    using type = ResultType(ArgsType...);
    typedef ResultType result_type;

    virtual ~FunctionBase() noexcept = 0;

    virtual result_type invoke(ArgsType&&... __args) const = 0;
    virtual result_type invoke(ArgsType&&... __args) = 0;

    virtual const void* address() const noexcept = 0;
    virtual void* address() noexcept = 0;
    virtual size_t size() const noexcept = 0;

    virtual bool clone(FunctionBase<ResultType(ArgsType...)>*& pOut, void* pBuf) const noexcept = 0;
};



/*-------------------------------------
 * Metadata without arguments
-------------------------------------*/
template <typename ResultType>
class FunctionBase<ResultType()>
{
public:
    using type = ResultType();
    typedef ResultType result_type;

    virtual ~FunctionBase() noexcept = 0;

    virtual result_type invoke() const = 0;
    virtual result_type invoke() = 0;

    virtual const void* address() const noexcept = 0;
    virtual void* address() noexcept = 0;
    virtual size_t size() const noexcept = 0;

    virtual bool clone(FunctionBase<ResultType()>*& pOut, void* pBuf) const noexcept = 0;
};


/*-----------------------------------------------------------------------------
 * Function storage wrapper
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Base Template
-------------------------------------*/
template <typename T, typename U>
class FunctionType;



/*-------------------------------------
 * Function storage with arguments
-------------------------------------*/
template <typename StorageType, typename ResultType, typename... ArgsType>
class FunctionType<StorageType, ResultType(ArgsType...)> : public FunctionBase<ResultType(ArgsType...)>
{
public:
    using type = FunctionBase<ResultType(ArgsType...)>::type;
    using result_type = FunctionBase<ResultType(ArgsType...)>::result_type;

private:
    StorageType mFunction;

public:
    virtual ~FunctionType() noexcept override;
    FunctionType() noexcept;
    explicit FunctionType(const FunctionType& f) noexcept;
    explicit FunctionType(FunctionType&& f) noexcept;
    explicit FunctionType(StorageType&& f) noexcept;

    FunctionType& operator=(const FunctionType& f) noexcept;
    FunctionType& operator=(FunctionType&& f) noexcept;
    FunctionType& operator=(StorageType&& f) noexcept;

    virtual result_type invoke(ArgsType&&... args) const override;
    virtual result_type invoke(ArgsType&&... args) override;

    virtual const void* address() const noexcept override;
    virtual void* address() noexcept override;
    virtual size_t size() const noexcept override;

    virtual bool clone(FunctionBase<ResultType(ArgsType...)>*& pOut, void* pBuf) const noexcept override;
    static FunctionBase<ResultType(ArgsType...)>* clone_unchecked(void* pBuf, StorageType&& func) noexcept;
};



/*-------------------------------------
 * Function Storage without arguments
-------------------------------------*/
template <typename StorageType, typename ResultType>
class FunctionType<StorageType, ResultType()> : public FunctionBase<ResultType()>
{
public:
    using type = FunctionBase<ResultType()>::type;
    using result_type = FunctionBase<ResultType()>::result_type;

private:
    StorageType mFunction;

public:
    virtual ~FunctionType() noexcept override;
    FunctionType() noexcept;
    explicit FunctionType(const FunctionType& f) noexcept;
    explicit FunctionType(FunctionType&& f) noexcept;
    explicit FunctionType(StorageType&& f) noexcept;

    FunctionType& operator=(const FunctionType& f) noexcept;
    FunctionType& operator=(FunctionType&& f) noexcept;
    FunctionType& operator=(StorageType&& f) noexcept;

    virtual result_type invoke() const override;
    virtual result_type invoke() override;

    virtual const void* address() const noexcept override;
    virtual void* address() noexcept override;
    virtual size_t size() const noexcept override;

    virtual bool clone(FunctionBase<ResultType()>*& pOut, void* pBuf) const noexcept override;
    static FunctionBase<ResultType()>* clone_unchecked(void* pBuf, StorageType&& func) noexcept;
};



/*-----------------------------------------------------------------------------
 * Function callable container
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Callable base template
-------------------------------------*/
template <typename T>
class CallableType;



/*-------------------------------------
 * Callable type with arguments
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
class alignas(alignof(FunctionBase<ResultType(ArgsType...)>*)) CallableType<ResultType(ArgsType...)>
{
public:
    friend class Function<ResultType(ArgsType...)>;
    using wrapper_type = FunctionBase<ResultType(ArgsType...)>;
    using target_type = FunctionBase<ResultType(ArgsType...)>::type;
    using result_type = FunctionBase<ResultType(ArgsType...)>::result_type;

    enum : size_t
    {
        internal_buffer_size = sizeof(FunctionBase<ResultType(ArgsType...)>*) * 3,
        internal_alignment = alignof(FunctionBase<ResultType(ArgsType...)>*)
    };

private:
    FunctionBase<ResultType(ArgsType...)>* mFunction;
    unsigned char mBuffer[internal_buffer_size];

public:
    ~CallableType() noexcept;
    CallableType() noexcept;
    CallableType(const CallableType& c) noexcept;
    CallableType(CallableType&& c) noexcept;

    CallableType& operator=(const CallableType& c) noexcept;
    CallableType& operator=(CallableType&& c) noexcept;

private:
    template <typename Callable>
    bool _init(Callable&& fp) noexcept;
    void _reset() noexcept;
    void _swap(CallableType& c) noexcept;

    result_type invoke(ArgsType&&... args) const;
    result_type invoke(ArgsType&&... args);

    const target_type* target() const noexcept;
    target_type* target() noexcept;
    size_t target_size() const noexcept;
};



/*-------------------------------------
 * Callable type without arguments
-------------------------------------*/
template <typename ResultType>
class alignas(alignof(FunctionBase<ResultType()>*)) CallableType<ResultType()>
{
public:
    friend class Function<ResultType()>;
    using wrapper_type = FunctionBase<ResultType()>;
    using target_type = FunctionBase<ResultType()>::type;
    using result_type = FunctionBase<ResultType()>::result_type;

    enum : size_t
    {
        internal_buffer_size = sizeof(FunctionBase<ResultType()>*) * 3,
        internal_alignment = alignof(FunctionBase<ResultType()>*)
    };

private:
    FunctionBase<ResultType()>* mFunction;
    unsigned char mBuffer[internal_buffer_size];

public:
    ~CallableType() noexcept;
    CallableType() noexcept;
    CallableType(const CallableType& c) noexcept;
    CallableType(CallableType&& c) noexcept;

    CallableType& operator=(const CallableType& c) noexcept;
    CallableType& operator=(CallableType&& c) noexcept;

private:
    template <typename Callable>
    bool _init(Callable&& fp) noexcept;
    void _reset() noexcept;
    void _swap(CallableType& c) noexcept;

    result_type invoke() const;
    result_type invoke();

    const target_type* target() const noexcept;
    target_type* target() noexcept;
    size_t target_size() const noexcept;
};



/*-----------------------------------------------------------------------------
 * Function class (with arguments)
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Base Function template
-------------------------------------*/
template <typename Empty>
class Function;



/*-------------------------------------
 * Function type with arguments
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
class Function<ResultType(ArgsType...)>
{
public:
    typedef FunctionBase<ResultType(ArgsType...)> type;
    typedef ResultType result_type;

private:
    CallableType<ResultType(ArgsType...)> mFunction;

    void reset() noexcept;

public:
    ~Function();
    Function() noexcept;
    Function(const Function&) noexcept;
    Function(Function&&) noexcept;

    template <typename Callable>
    Function(Callable&&) noexcept;

    Function& operator=(const Function&) noexcept;
    Function& operator=(Function&&) noexcept;

    template <typename Callable>
    Function& operator=(Callable&&) noexcept;

    explicit operator bool() const noexcept;
    result_type operator()(ArgsType&&... args) const;
    result_type operator()(ArgsType&&... args);

    const type* target() const noexcept;
    type* target() noexcept;
    size_t target_size() const noexcept;
};



/*-------------------------------------
 * Function type without arguments
-------------------------------------*/
template <typename ResultType>
class Function<ResultType()>
{
public:
    typedef FunctionBase<ResultType()>::type type;
    typedef ResultType result_type;

private:
    CallableType<ResultType()> mFunction;

    void reset() noexcept;

public:
    ~Function();
    Function() noexcept;
    Function(const Function&) noexcept;
    Function(Function&&) noexcept;

    template <typename Callable>
    Function(Callable&&) noexcept;

    Function& operator=(const Function&) noexcept;
    Function& operator=(Function&&) noexcept;

    template <typename Callable>
    Function& operator=(Callable&&) noexcept;

    explicit operator bool() const noexcept;
    result_type operator()() const;
    result_type operator()();

    const type* target() const noexcept;
    type* target() noexcept;
    size_t target_size() const noexcept;
};



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/FunctionImpl.hpp"

#endif /* LS_UTILS_FUNCTION_HPP */
