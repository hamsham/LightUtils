/*
 * File:   FunctionImpl.hpp
 * Author: miles
 * Created on October 27, 2025, at 9:44 a.m.
 */

#ifndef LS_UTILS_FUNCTION_IMPL_HPP
#define LS_UTILS_FUNCTION_IMPL_HPP

#include <memory> // std::nothrow

#include "lightsky/setup/Api.h" // LS_IMPERATIVE
#include "lightsky/setup/Types.h" // move(), forward()

namespace ls
{
namespace utils
{

/*-----------------------------------------------------------------------------
 * Function metadata with arguments
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
FunctionBase<ResultType(ArgsType...)>::~FunctionBase() noexcept
{
}



/*-----------------------------------------------------------------------------
 * Function metadata without arguments
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename ResultType>
FunctionBase<ResultType()>::~FunctionBase() noexcept
{
}


/*-----------------------------------------------------------------------------
 * Function storage wrapper with arguments
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename StorageType, typename ResultType, typename... ArgsType>
FunctionType<StorageType, ResultType(ArgsType...)>::~FunctionType() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename StorageType, typename ResultType, typename... ArgsType>
FunctionType<StorageType, ResultType(ArgsType...)>::FunctionType() noexcept :
    mFunction{}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <typename StorageType, typename ResultType, typename... ArgsType>
FunctionType<StorageType, ResultType(ArgsType...)>::FunctionType(const FunctionType& f) noexcept :
    mFunction{f.mFunction}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <typename StorageType, typename ResultType, typename... ArgsType>
FunctionType<StorageType, ResultType(ArgsType...)>::FunctionType(FunctionType&& f) noexcept :
    mFunction{ls::setup::move(f.mFunction)}
{}



/*-------------------------------------
 * Value Constructor
-------------------------------------*/
template <typename StorageType, typename ResultType, typename... ArgsType>
FunctionType<StorageType, ResultType(ArgsType...)>::FunctionType(StorageType&& f) noexcept :
    mFunction{ls::setup::forward<StorageType>(f)}
{}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template <typename StorageType, typename ResultType, typename... ArgsType>
FunctionType<StorageType, ResultType(ArgsType...)>& FunctionType<StorageType, ResultType(ArgsType...)>::operator=(const FunctionType& f) noexcept
{
    if (this != &f)
    {
        mFunction = f.mFunction;
    }

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <typename StorageType, typename ResultType, typename... ArgsType>
FunctionType<StorageType, ResultType(ArgsType...)>& FunctionType<StorageType, ResultType(ArgsType...)>::operator=(FunctionType&& f) noexcept
{
    if (this != &f)
    {
        mFunction = ls::setup::move(f.mFunction); f.mFunction = nullptr;
    }

    return *this;
}



/*-------------------------------------
 * Value Assignment
-------------------------------------*/
template <typename StorageType, typename ResultType, typename... ArgsType>
FunctionType<StorageType, ResultType(ArgsType...)>& FunctionType<StorageType, ResultType(ArgsType...)>::operator=(StorageType&& f) noexcept
{
    mFunction = ls::setup::forward<StorageType>(f);
    return *this;
}



/*-------------------------------------
 * Function Call Operator (const)
-------------------------------------*/
template <typename StorageType, typename ResultType, typename... ArgsType>
typename FunctionType<StorageType, ResultType(ArgsType...)>::result_type
LS_IMPERATIVE FunctionType<StorageType, ResultType(ArgsType...)>::invoke(ArgsType&&... args) const
{
    return mFunction(ls::setup::forward<ArgsType>(args)...);
}



/*-------------------------------------
 * Function Call Operator
-------------------------------------*/
template <typename StorageType, typename ResultType, typename... ArgsType>
typename FunctionType<StorageType, ResultType(ArgsType...)>::result_type
LS_IMPERATIVE FunctionType<StorageType, ResultType(ArgsType...)>::invoke(ArgsType&&... args)
{
    return mFunction(ls::setup::forward<ArgsType>(args)...);
}



/*-------------------------------------
 * Get Function Base Address (const)
-------------------------------------*/
template <typename StorageType, typename ResultType, typename... ArgsType>
const void* LS_IMPERATIVE FunctionType<StorageType, ResultType(ArgsType...)>::address() const noexcept
{
    return &mFunction;
}



/*-------------------------------------
 * Get Function Base Address (const)
-------------------------------------*/
template <typename StorageType, typename ResultType, typename... ArgsType>
void* LS_IMPERATIVE FunctionType<StorageType, ResultType(ArgsType...)>::address() noexcept
{
    return &mFunction;
}



/*-------------------------------------
 * Function Size
-------------------------------------*/
template <typename StorageType, typename ResultType, typename... ArgsType>
size_t LS_IMPERATIVE FunctionType<StorageType, ResultType(ArgsType...)>::size() const noexcept
{
    return sizeof(StorageType);
}



/*-------------------------------------
 * Duplication
-------------------------------------*/
template <typename StorageType, typename ResultType, typename... ArgsType>
bool FunctionType<StorageType, ResultType(ArgsType...)>::clone(FunctionBase<ResultType(ArgsType...)>*& pOut, void* pBuf) const noexcept
{
    constexpr size_t maxSlackSpace = CallableType<ResultType(ArgsType...)>::internal_buffer_size;
    if (sizeof(FunctionType<StorageType, ResultType(ArgsType...)>) <= maxSlackSpace)
    {
        pOut = new(pBuf) FunctionType<StorageType, ResultType(ArgsType...)>{*this};
    }
    else
    {
        pOut = new(std::nothrow) FunctionType<StorageType, ResultType(ArgsType...)>{*this};
    }

    return pOut != nullptr;
}



/*-------------------------------------
 * Duplication (static)
-------------------------------------*/
template <typename StorageType, typename ResultType, typename... ArgsType>
FunctionBase<ResultType(ArgsType...)>* FunctionType<StorageType, ResultType(ArgsType...)>::clone_unchecked(void* pBuf, StorageType&& func) noexcept
{
    constexpr size_t maxSlackSpace = CallableType<ResultType(ArgsType...)>::internal_buffer_size;
    FunctionBase<ResultType(ArgsType...)>* pOut;

    if (sizeof(FunctionType<StorageType, ResultType(ArgsType...)>) <= maxSlackSpace)
    {
        pOut = new(pBuf) FunctionType<StorageType, ResultType(ArgsType...)>{ls::setup::forward<StorageType>(func)};
    }
    else
    {
        pOut = new(std::nothrow) FunctionType<StorageType, ResultType(ArgsType...)>{ls::setup::forward<StorageType>(func)};
    }

    return pOut;
}



/*-----------------------------------------------------------------------------
 * Function storage wrapper without arguments
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename StorageType, typename ResultType>
FunctionType<StorageType, ResultType()>::~FunctionType() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename StorageType, typename ResultType>
FunctionType<StorageType, ResultType()>::FunctionType() noexcept :
    mFunction{}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <typename StorageType, typename ResultType>
FunctionType<StorageType, ResultType()>::FunctionType(const FunctionType& f) noexcept :
    mFunction{f.mFunction}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <typename StorageType, typename ResultType>
FunctionType<StorageType, ResultType()>::FunctionType(FunctionType&& f) noexcept :
    mFunction{ls::setup::move(f.mFunction)}
{}



/*-------------------------------------
 * Value Constructor
-------------------------------------*/
template <typename StorageType, typename ResultType>
FunctionType<StorageType, ResultType()>::FunctionType(StorageType&& f) noexcept :
    mFunction{ls::setup::forward<StorageType>(f)}
{}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template <typename StorageType, typename ResultType>
FunctionType<StorageType, ResultType()>& FunctionType<StorageType, ResultType()>::operator=(const FunctionType& f) noexcept
{
    if (this != &f)
    {
        mFunction = f.mFunction;
    }

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <typename StorageType, typename ResultType>
FunctionType<StorageType, ResultType()>& FunctionType<StorageType, ResultType()>::operator=(FunctionType&& f) noexcept
{
    if (this != &f)
    {
        mFunction = ls::setup::move(f.mFunction); f.mFunction = nullptr;
    }

    return *this;
}



/*-------------------------------------
 * Value Assignment
-------------------------------------*/
template <typename StorageType, typename ResultType>
FunctionType<StorageType, ResultType()>& FunctionType<StorageType, ResultType()>::operator=(StorageType&& f) noexcept
{
    mFunction = ls::setup::forward<StorageType>(f);
    return *this;
}



/*-------------------------------------
 * Function Call Operator (const)
-------------------------------------*/
template <typename StorageType, typename ResultType>
typename FunctionType<StorageType, ResultType()>::result_type
LS_IMPERATIVE FunctionType<StorageType, ResultType()>::invoke() const
{
    return mFunction();
}



/*-------------------------------------
 * Function Call Operator
-------------------------------------*/
template <typename StorageType, typename ResultType>
typename FunctionType<StorageType, ResultType()>::result_type
LS_IMPERATIVE FunctionType<StorageType, ResultType()>::invoke()
{
    return mFunction();
}



/*-------------------------------------
 * Get Function Base Address (const)
-------------------------------------*/
template <typename StorageType, typename ResultType>
const void* LS_IMPERATIVE FunctionType<StorageType, ResultType()>::address() const noexcept
{
    return &mFunction;
}



/*-------------------------------------
 * Get Function Base Address (const)
-------------------------------------*/
template <typename StorageType, typename ResultType>
void* LS_IMPERATIVE FunctionType<StorageType, ResultType()>::address() noexcept
{
    return &mFunction;
}



/*-------------------------------------
 * Function Size
-------------------------------------*/
template <typename StorageType, typename ResultType>
size_t LS_IMPERATIVE FunctionType<StorageType, ResultType()>::size() const noexcept
{
    return sizeof(StorageType);
}



/*-------------------------------------
 * Duplication
-------------------------------------*/
template <typename StorageType, typename ResultType>
bool FunctionType<StorageType, ResultType()>::clone(FunctionBase<ResultType()>*& pOut, void* pBuf) const noexcept
{
    constexpr size_t maxSlackSpace = CallableType<ResultType()>::internal_buffer_size;
    if (sizeof(FunctionType<StorageType, ResultType()>) <= maxSlackSpace)
    {
        pOut = new(pBuf) FunctionType<StorageType, ResultType()>{*this};
    }
    else
    {
        pOut = new(std::nothrow) FunctionType<StorageType, ResultType()>{*this};
    }

    return pOut != nullptr;
}



/*-------------------------------------
 * Duplication (static)
-------------------------------------*/
template <typename StorageType, typename ResultType>
FunctionBase<ResultType()>* FunctionType<StorageType, ResultType()>::clone_unchecked(void* pBuf, StorageType&& func) noexcept
{
    constexpr size_t maxSlackSpace = CallableType<ResultType()>::internal_buffer_size;
    FunctionBase<ResultType()>* pOut;

    if (sizeof(FunctionType<StorageType, ResultType()>) <= maxSlackSpace)
    {
        pOut = new(pBuf) FunctionType<StorageType, ResultType()>{ls::setup::forward<StorageType>(func)};
    }
    else
    {
        pOut = new(std::nothrow) FunctionType<StorageType, ResultType()>{ls::setup::forward<StorageType>(func)};
    }

    return pOut;
}



/*-----------------------------------------------------------------------------
 * Function callable container with arguments
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
CallableType<ResultType(ArgsType...)>::~CallableType() noexcept
{
    if ((void*)mFunction == (void*)mBuffer)
    {
        mFunction->~FunctionBase<ResultType(ArgsType...)>();
    }
    else
    {
        delete mFunction;
    }
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
CallableType<ResultType(ArgsType...)>::CallableType() noexcept :
    mFunction{nullptr},
    mBuffer{}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
CallableType<ResultType(ArgsType...)>::CallableType(const CallableType& c) noexcept :
    CallableType{}
{
    if (c.mFunction != nullptr)
    {
        c.mFunction->clone(mFunction, mBuffer);
    }
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
CallableType<ResultType(ArgsType...)>::CallableType(CallableType&& c) noexcept
{
    if ((void*)c.mFunction != (void*)c.mBuffer)
    {
        mFunction = c.mFunction;
    }
    else
    {
        mFunction = reinterpret_cast<FunctionBase<ResultType(ArgsType...)>*>(mBuffer);
    }
    c.mFunction = nullptr;

    ls::utils::fast_memcpy(mBuffer, c.mBuffer, sizeof(mBuffer));
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
CallableType<ResultType(ArgsType...)>& CallableType<ResultType(ArgsType...)>::operator=(const CallableType& c) noexcept
{
    if (this != &c)
    {
        _reset();

        if (c.mFunction != nullptr)
        {
            c.mFunction->clone(mFunction, mBuffer);
        }
    }

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
CallableType<ResultType(ArgsType...)>& CallableType<ResultType(ArgsType...)>::operator=(CallableType&& c) noexcept
{
    if (this != &c)
    {
        if (mFunction != nullptr)
        {
            _reset();
        }

        if ((void*)c.mFunction != (void*)c.mBuffer)
        {
            mFunction = c.mFunction;
        }
        else
        {
            mFunction = reinterpret_cast<FunctionBase<ResultType(ArgsType...)>*>(mBuffer);
        }

        c.mFunction = nullptr;
        ls::utils::fast_memcpy(mBuffer, c.mBuffer, sizeof(mBuffer));
    }

    return *this;
}



/*-------------------------------------
 * Function Initialization
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
template <typename Callable>
bool CallableType<ResultType(ArgsType...)>::_init(Callable&& fp) noexcept
{
    _reset();
    mFunction = FunctionType<Callable, ResultType(ArgsType...)>::clone_unchecked(mBuffer, ls::setup::forward<Callable>(fp));
    return mFunction != nullptr;
}



/*-------------------------------------
 * Function Cleanup
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
void CallableType<ResultType(ArgsType...)>::_reset() noexcept
{
    if (mFunction != nullptr)
    {
        if ((void*)mFunction == (void*)mBuffer)
        {
            mFunction->~FunctionBase<ResultType(ArgsType...)>();
            ls::utils::fast_memset(mBuffer, '\0', sizeof(mBuffer));
        }
        else
        {
            delete mFunction;
        }

        mFunction = nullptr;
    }
}



/*-------------------------------------
 * Value Swap
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
void CallableType<ResultType(ArgsType...)>::_swap(CallableType& c) noexcept
{
    CallableType tmp = std::move(*this);
    *this = std::move(c);
    c = std::move(tmp);
}



/*-------------------------------------
 * Function Call Operator (const)
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
typename CallableType<ResultType(ArgsType...)>::result_type
LS_IMPERATIVE CallableType<ResultType(ArgsType...)>::invoke(ArgsType&&... args) const
{
    return mFunction->invoke(ls::setup::forward<ArgsType>(args)...);
}



/*-------------------------------------
 * Function Call Operator
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
typename CallableType<ResultType(ArgsType...)>::result_type
LS_IMPERATIVE CallableType<ResultType(ArgsType...)>::invoke(ArgsType&&... args)
{
    return mFunction->invoke(ls::setup::forward<ArgsType>(args)...);
}



/*-------------------------------------
 * Target Function Address (const)
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
const typename CallableType<ResultType(ArgsType...)>::target_type*
LS_IMPERATIVE CallableType<ResultType(ArgsType...)>::target() const noexcept
{
    return mFunction;
}



/*-------------------------------------
 * Target Function Address
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
typename CallableType<ResultType(ArgsType...)>::target_type*
LS_IMPERATIVE CallableType<ResultType(ArgsType...)>::target() noexcept
{
    return mFunction;
}



/*-------------------------------------
 * Function Size
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
size_t LS_IMPERATIVE CallableType<ResultType(ArgsType...)>::target_size() const noexcept
{
    return mFunction ? mFunction->size() : 0;
}



/*-----------------------------------------------------------------------------
 * Function callable container without arguments
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename ResultType>
CallableType<ResultType()>::~CallableType() noexcept
{
    if ((void*)mFunction == (void*)mBuffer)
    {
        mFunction->~FunctionBase<ResultType()>();
    }
    else
    {
        delete mFunction;
    }
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename ResultType>
CallableType<ResultType()>::CallableType() noexcept :
    mFunction{nullptr},
    mBuffer{}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <typename ResultType>
CallableType<ResultType()>::CallableType(const CallableType& c) noexcept :
    CallableType{}
{
    if (c.mFunction != nullptr)
    {
        c.mFunction->clone(mFunction, mBuffer);
    }
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <typename ResultType>
CallableType<ResultType()>::CallableType(CallableType&& c) noexcept
{
    if ((void*)c.mFunction != (void*)c.mBuffer)
    {
        mFunction = c.mFunction;
    }
    else
    {
        mFunction = reinterpret_cast<FunctionBase<ResultType()>*>(mBuffer);
    }
    c.mFunction = nullptr;

    ls::utils::fast_memcpy(mBuffer, c.mBuffer, sizeof(mBuffer));
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template <typename ResultType>
CallableType<ResultType()>& CallableType<ResultType()>::operator=(const CallableType& c) noexcept
{
    if (this != &c)
    {
        _reset();

        if (c.mFunction != nullptr)
        {
            c.mFunction->clone(mFunction, mBuffer);
        }
    }

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <typename ResultType>
CallableType<ResultType()>& CallableType<ResultType()>::operator=(CallableType&& c) noexcept
{
    if (this != &c)
    {
        if (mFunction != nullptr)
        {
            _reset();
        }

        if ((void*)c.mFunction != (void*)c.mBuffer)
        {
            mFunction = c.mFunction;
        }
        else
        {
            mFunction = reinterpret_cast<FunctionBase<ResultType()>*>(mBuffer);
        }

        c.mFunction = nullptr;
        ls::utils::fast_memcpy(mBuffer, c.mBuffer, sizeof(mBuffer));
    }

    return *this;
}



/*-------------------------------------
 * Function Initialization
-------------------------------------*/
template <typename ResultType>
template <typename Callable>
bool CallableType<ResultType()>::_init(Callable&& fp) noexcept
{
    _reset();
    mFunction = FunctionType<Callable, ResultType()>::clone_unchecked(mBuffer, ls::setup::forward<Callable>(fp));
    return mFunction != nullptr;
}



/*-------------------------------------
 * Function Cleanup
-------------------------------------*/
template <typename ResultType>
void CallableType<ResultType()>::_reset() noexcept
{
    if (mFunction != nullptr)
    {
        if ((void*)mFunction == (void*)mBuffer)
        {
            mFunction->~FunctionBase<ResultType()>();
            ls::utils::fast_memset(mBuffer, '\0', sizeof(mBuffer));
        }
        else
        {
            delete mFunction;
        }

        mFunction = nullptr;
    }
}



/*-------------------------------------
 * Value Swap
-------------------------------------*/
template <typename ResultType>
void CallableType<ResultType()>::_swap(CallableType& c) noexcept
{
    CallableType tmp = std::move(*this);
    *this = std::move(c);
    c = std::move(tmp);
}



/*-------------------------------------
 * Function Call Operator (const)
-------------------------------------*/
template <typename ResultType>
typename CallableType<ResultType()>::result_type
LS_IMPERATIVE CallableType<ResultType()>::invoke() const
{
    return mFunction->invoke();
}



/*-------------------------------------
 * Function Call Operator
-------------------------------------*/
template <typename ResultType>
typename CallableType<ResultType()>::result_type
LS_IMPERATIVE CallableType<ResultType()>::invoke()
{
    return mFunction->invoke();
}



/*-------------------------------------
 * Target Function Address (const)
-------------------------------------*/
template <typename ResultType>
const typename CallableType<ResultType()>::target_type*
LS_IMPERATIVE CallableType<ResultType()>::target() const noexcept
{
    return mFunction;
}



/*-------------------------------------
 * Target Function Address
-------------------------------------*/
template <typename ResultType>
typename CallableType<ResultType()>::target_type*
LS_IMPERATIVE CallableType<ResultType()>::target() noexcept
{
    return mFunction;
}



/*-------------------------------------
 * Function Size
-------------------------------------*/
template <typename ResultType>
size_t LS_IMPERATIVE CallableType<ResultType()>::target_size() const noexcept
{
    return mFunction ? mFunction->size() : 0;
}



/*-----------------------------------------------------------------------------
 * Function class with arguments
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Value Cleanup
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
void Function<ResultType(ArgsType...)>::reset() noexcept
{
    mFunction._reset();
}



/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
Function<ResultType(ArgsType...)>::~Function()
{
    reset();
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
Function<ResultType(ArgsType...)>::Function() noexcept :
    mFunction{}
{}



/*-------------------------------------
 * Value Constructor
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
template <typename Callable>
Function<ResultType(ArgsType...)>::Function(Callable&& func) noexcept :
    mFunction{}
{
    mFunction._init(ls::setup::forward<Callable>(func));
}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
Function<ResultType(ArgsType...)>::Function(const Function& func) noexcept :
    mFunction{func.mFunction}
{
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
Function<ResultType(ArgsType...)>::Function(Function&& func) noexcept :
    mFunction{ls::setup::move(func.mFunction)}
{
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
Function<ResultType(ArgsType...)>& Function<ResultType(ArgsType...)>::operator=(const Function& func) noexcept
{
    if (this != &func)
    {
        mFunction = func.mFunction;
    }

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
Function<ResultType(ArgsType...)>& Function<ResultType(ArgsType...)>::operator=(Function&& func) noexcept
{
    if (this != &func)
    {
        mFunction = ls::setup::move(func.mFunction);
    }

    return *this;
}



/*-------------------------------------
 * Value Assignment
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
template <typename Callable>
Function<ResultType(ArgsType...)>& Function<ResultType(ArgsType...)>::operator=(Callable&& func) noexcept
{
    mFunction._init(ls::setup::forward<Callable>(func));
    return *this;
}



/*-------------------------------------
 * Validation Check
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
LS_IMPERATIVE Function<ResultType(ArgsType...)>::operator bool() const noexcept
{
    return mFunction.mFunction != nullptr;
}



/*-------------------------------------
 * Function Call Operator (const)
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
typename Function<ResultType(ArgsType...)>::result_type
LS_IMPERATIVE Function<ResultType(ArgsType...)>::operator()(ArgsType&&... args) const
{
    return mFunction.invoke(ls::setup::forward<ArgsType>(args)...);
}



/*-------------------------------------
 * Function Call Operator
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
typename Function<ResultType(ArgsType...)>::result_type
LS_IMPERATIVE Function<ResultType(ArgsType...)>::operator()(ArgsType&&... args)
{
    return mFunction.invoke(ls::setup::forward<ArgsType>(args)...);
}



/*-------------------------------------
 * Function Address (const)
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
const typename Function<ResultType(ArgsType...)>::type*
LS_IMPERATIVE Function<ResultType(ArgsType...)>::target() const noexcept
{
    return mFunction.target();
}



/*-------------------------------------
 * Function Address
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
typename Function<ResultType(ArgsType...)>::type*
LS_IMPERATIVE Function<ResultType(ArgsType...)>::target() noexcept
{
    return mFunction.target();
}



/*-------------------------------------
 * Function Size
-------------------------------------*/
template <typename ResultType, typename... ArgsType>
size_t LS_IMPERATIVE Function<ResultType(ArgsType...)>::target_size() const noexcept
{
    return mFunction.target_size();
}



/*-----------------------------------------------------------------------------
 * Function class without arguments
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Value Cleanup
-------------------------------------*/
template <typename ResultType>
void Function<ResultType()>::reset() noexcept
{
    mFunction._reset();
}



/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename ResultType>
Function<ResultType()>::~Function()
{
    reset();
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename ResultType>
Function<ResultType()>::Function() noexcept :
    mFunction{}
{}



/*-------------------------------------
 * Value Constructor
-------------------------------------*/
template <typename ResultType>
template <typename Callable>
Function<ResultType()>::Function(Callable&& func) noexcept :
    mFunction{}
{
    mFunction._init(ls::setup::forward<Callable>(func));
}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <typename ResultType>
Function<ResultType()>::Function(const Function& func) noexcept :
    mFunction{func.mFunction}
{
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <typename ResultType>
Function<ResultType()>::Function(Function&& func) noexcept :
    mFunction{ls::setup::move(func.mFunction)}
{
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template <typename ResultType>
Function<ResultType()>& Function<ResultType()>::operator=(const Function& func) noexcept
{
    if (this != &func)
    {
        mFunction = func.mFunction;
    }

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <typename ResultType>
Function<ResultType()>& Function<ResultType()>::operator=(Function&& func) noexcept
{
    if (this != &func)
    {
        mFunction = ls::setup::move(func.mFunction);
    }

    return *this;
}



/*-------------------------------------
 * Value Assignment
-------------------------------------*/
template <typename ResultType>
template <typename Callable>
Function<ResultType()>& Function<ResultType()>::operator=(Callable&& func) noexcept
{
    mFunction._init(ls::setup::forward<Callable>(func));
    return *this;
}



/*-------------------------------------
 * Validation Check
-------------------------------------*/
template <typename ResultType>
LS_IMPERATIVE Function<ResultType()>::operator bool() const noexcept
{
    return mFunction.mFunction != nullptr;
}



/*-------------------------------------
 * Function Call Operator (const)
-------------------------------------*/
template <typename ResultType>
typename Function<ResultType()>::result_type
LS_IMPERATIVE Function<ResultType()>::operator()() const
{
    return mFunction.invoke();
}



/*-------------------------------------
 * Function Call Operator
-------------------------------------*/
template <typename ResultType>
typename Function<ResultType()>::result_type
LS_IMPERATIVE Function<ResultType()>::operator()()
{
    return mFunction.invoke();
}



/*-------------------------------------
 * Function Address (const)
-------------------------------------*/
template <typename ResultType>
const typename Function<ResultType()>::type*
LS_IMPERATIVE Function<ResultType()>::target() const noexcept
{
    return mFunction.target();
}



/*-------------------------------------
 * Function Address
-------------------------------------*/
template <typename ResultType>
typename Function<ResultType()>::type*
LS_IMPERATIVE Function<ResultType()>::target() noexcept
{
    return mFunction.target();
}



/*-------------------------------------
 * Function Size
-------------------------------------*/
template <typename ResultType>
size_t LS_IMPERATIVE Function<ResultType()>::target_size() const noexcept
{
    return mFunction.target_size();
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_FUNCTION_IMPL_HPP */
