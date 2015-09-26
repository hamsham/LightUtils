
namespace ls {
namespace utils {

/*-------------------------------------
    Static Methods
-------------------------------------*/
/*-------------------------------------
    tuple_t<data_t...>::constructObjects (Sentinel)
-------------------------------------*/
template <typename... data_t>
template <typename arg_t> constexpr
bool tuple_t<data_t...>::constructObjects(char* buffer, unsigned offset, arg_t*) {
    return new(buffer+offset) arg_t{} != nullptr;
}

/*-------------------------------------
    tuple_t<data_t...>::constructObjects
-------------------------------------*/
template <typename... data_t>
template <typename arg_t, typename... args_t> constexpr
bool tuple_t<data_t...>::constructObjects(char* buffer, unsigned offset, arg_t*, args_t*... args) {
    return new(buffer+offset) arg_t{} != nullptr
        ? constructObjects<args_t...>(buffer, offset+sizeof(arg_t), args...)
        : false;
}

/*-------------------------------------
    tuple_t<data_t...>::destroyObjects
-------------------------------------*/
template <typename... data_t>
template <typename arg_t> constexpr
bool tuple_t<data_t...>::destroyObjects(char* buffer, unsigned offset, arg_t*) {
    return std::is_destructible<arg_t>()
        ? reinterpret_cast<arg_t*>(buffer+offset)->~arg_t(), true
        : throw;
}

/*-------------------------------------
    tuple_t<data_t...>::destroyObjects
-------------------------------------*/
template <typename... data_t>
template <typename arg_t, typename... args_t> inline
bool tuple_t<data_t...>::destroyObjects(char* buffer, unsigned offset, arg_t*, args_t*... args) {
    if (std::is_destructible<arg_t>()) {
        reinterpret_cast<arg_t*>(buffer+offset)->~arg_t();
    }
    destroyObjects<args_t...>(buffer, offset+sizeof(arg_t), args...);
    return true;
}

/*-------------------------------------
    Movement Methods
-------------------------------------*/
/*-------------------------------------
    tuple_t<data_t...>::copyObjects (Sentinel)
-------------------------------------*/
template <typename... data_t>
template <typename arg_t> inline
void tuple_t<data_t...>::copyObjects(const tuple_t<data_t...>& agg, char* buffer, unsigned offset, arg_t*) {
    static_assert(std::is_copy_assignable<arg_t>(), "Aggregated objects must have a copy operator available.");
    
    arg_t* pArg = (arg_t*)(buffer+offset);
    *pArg = *agg.getObject<arg_t>();
}

/*-------------------------------------
    tuple_t<data_t...>::copyObjects
-------------------------------------*/
template <typename... data_t>
template <typename arg_t, typename... args_t> inline
void tuple_t<data_t...>::copyObjects(const tuple_t<data_t...>& agg, char* buffer, unsigned offset, arg_t*, args_t*... args) {
    static_assert(std::is_copy_assignable<arg_t>(), "Aggregated objects must have a copy operator available.");
    
    arg_t* pArg = (arg_t*)(buffer+offset);
    *pArg =*agg.getObject<arg_t>();
    copyObjects(agg, buffer, offset+sizeof(arg_t), args...);
}

/*-------------------------------------
    tuple_t<data_t...>::moveObjects
-------------------------------------*/
template <typename... data_t>
template <typename arg_t> inline
void tuple_t<data_t...>::moveObjects(tuple_t<data_t...>&& agg, char* buffer, unsigned offset, arg_t*) {
    static_assert(std::is_copy_assignable<arg_t>(), "Aggregated objects must have a move operator available.");
    
    arg_t* pArg = (arg_t*)(buffer+offset);
    *pArg = std::move(*agg.getObject<arg_t>());
}

/*-------------------------------------
    tuple_t<data_t...>::moveObjects
-------------------------------------*/
template <typename... data_t>
template <typename arg_t, typename... args_t> inline
void tuple_t<data_t...>::moveObjects(tuple_t<data_t...>&& agg, char* buffer, unsigned offset, arg_t*, args_t*... args) {
    static_assert(std::is_copy_assignable<arg_t>(), "Aggregated objects must have a move operator available.");
    
    arg_t* pArg = (arg_t*)(buffer+offset);
    *pArg = std::move(*agg.getObject<arg_t>());
    moveObjects(std::forward<tuple_t<data_t...>>(agg), buffer, offset+sizeof(arg_t), args...);
}

/*-------------------------------------
    Construction and Destruction
-------------------------------------*/
/*-------------------------------------
    Private Constructor
-------------------------------------*/
template <typename... data_t>
constexpr tuple_t<data_t...>::tuple_t(bool)
{}

/*-------------------------------------
    Constructor
-------------------------------------*/
template <typename... data_t>
constexpr tuple_t<data_t...>::tuple_t() :
    tuple_t(constructObjects(dataBuffer, 0, ((data_t*)nullptr)...))
{}

/*-------------------------------------
    Copy Constructor
-------------------------------------*/
template <typename... data_t>
tuple_t<data_t...>::tuple_t(const tuple_t& a) :
    tuple_t{}
{
    copyObjects(a, dataBuffer, 0, ((data_t*)nullptr)...);
}

/*-------------------------------------
    Move Constructor
-------------------------------------*/
template <typename... data_t>
tuple_t<data_t...>::tuple_t(tuple_t&& a) :
    tuple_t{}
{
    moveObjects(std::forward<tuple_t<data_t...>>(a), dataBuffer, 0, ((data_t*)nullptr)...);
}

/*-------------------------------------
    Destructor
-------------------------------------*/
template <typename... data_t>
tuple_t<data_t...>::~tuple_t() {
    destroyObjects(dataBuffer, 0, ((data_t*)nullptr)...);
}

/*-------------------------------------
    Copy Operator
-------------------------------------*/
template <typename... data_t>
tuple_t<data_t...>& tuple_t<data_t...>::operator=(const tuple_t& a) {
    copyObjects(a, dataBuffer, 0, ((data_t*)nullptr)...);
    return *this;
}

/*-------------------------------------
    Move Operator
-------------------------------------*/
template <typename... data_t>
tuple_t<data_t...>& tuple_t<data_t...>::operator=(tuple_t&& a) {
    moveObjects(std::forward<tuple_t<data_t...>>(a), dataBuffer, 0, ((data_t*)nullptr)...);
    return *this;
}

/*-------------------------------------
    Get an object using an its type (const)
-------------------------------------*/
/*-------------------------------------
    Get the last object in the data buffer. (Sentinel)
-------------------------------------*/
template <typename... data_t>
template <typename request_t, typename arg_t> constexpr
const request_t* tuple_t<data_t...>::getObjectAtOffset(unsigned offset, arg_t*) const {
    return std::is_same<request_t, arg_t>()
        ? (const request_t*)(dataBuffer+offset)
        : nullptr;
}

/*-------------------------------------
    Get an object from the data buffer.
-------------------------------------*/
template <typename... data_t>
template <typename request_t, typename arg_t, typename... args_t> constexpr
const request_t* tuple_t<data_t...>::getObjectAtOffset(unsigned offset, arg_t*, args_t*...) const {
    return std::is_same<request_t, arg_t>()
        ? (const request_t*)(dataBuffer+offset)
        : getObjectAtOffset<request_t, args_t...>(sizeof(arg_t)+offset, ((args_t*)nullptr)...);
}

/*-------------------------------------
    Get an object contained within *this. This object is specified using
    template parameters.
-------------------------------------*/
template <typename... data_t>
template <typename request_t> constexpr
const request_t* tuple_t<data_t...>::getObject() const {
    return getObjectAtOffset<request_t, data_t...>(0, ((data_t*)nullptr)...);
}

/*-------------------------------------
    Get an object using an its type
-------------------------------------*/
/*-------------------------------------
    Get the last object in the data buffer.
-------------------------------------*/
template <typename... data_t>
template <typename request_t, typename arg_t> inline
request_t* tuple_t<data_t...>::getObjectAtOffset(unsigned offset, arg_t*) {
    return std::is_same<request_t, arg_t>()
        ? (request_t*)(dataBuffer+offset)
        : nullptr;
}

/*-------------------------------------
    Get an object from the data buffer.
-------------------------------------*/
template <typename... data_t>
template <typename request_t, typename arg_t, typename... args_t> inline
request_t* tuple_t<data_t...>::getObjectAtOffset(unsigned offset, arg_t*, args_t*...) {
    return std::is_same<request_t, arg_t>()
        ? (request_t*)(dataBuffer+offset)
        : getObjectAtOffset<request_t, args_t...>(sizeof(arg_t)+offset, ((args_t*)nullptr)...);
}

/*-------------------------------------
    Get an object contained within *this. This object is specified using
    template parameters.
-------------------------------------*/
template <typename... data_t>
template <typename request_t> inline
request_t* tuple_t<data_t...>::getObject() {
    return getObjectAtOffset<request_t, data_t...>(0, ((data_t*)nullptr)...);
}

/*-------------------------------------
    Get an object using an its index (const)
-------------------------------------*/
/*-------------------------------------
    Get the last object in the data buffer. (Sentinel)
-------------------------------------*/
template <typename... data_t>
template <typename arg_t> constexpr
const void* tuple_t<data_t...>::getObjectAtIndex(unsigned index, unsigned offset, arg_t*) const {
    return index == 0
        ? (const void*)(dataBuffer+offset)
        : nullptr;
}

/*-------------------------------------
    Get an object from the data buffer.
-------------------------------------*/
template <typename... data_t>
template <typename arg_t, typename... args_t> constexpr
const void* tuple_t<data_t...>::getObjectAtIndex(unsigned index, unsigned offset, arg_t*, args_t*...) const {
    return (index == 0)
        ? (const void*)(dataBuffer+offset)
        : getObjectAtIndex<args_t...>(index-1, sizeof(arg_t)+offset, ((args_t*)nullptr)...);
}

/*-------------------------------------
    Get an object contained within *this. This object is specified using
    an array-like index.
-------------------------------------*/
template <typename... data_t> constexpr
const void* tuple_t<data_t...>::getObject(unsigned index) const {
    return getObjectAtIndex<data_t...>(index, 0, ((data_t*)nullptr)...);
}

/*-------------------------------------
    Get an object using an its index
-------------------------------------*/
/*-------------------------------------
    Get the last object in the data buffer.
-------------------------------------*/
template <typename... data_t>
template <typename arg_t> inline
void* tuple_t<data_t...>::getObjectAtIndex(unsigned index, unsigned offset, arg_t*) {
    return index == 0
        ? (void*)(dataBuffer+offset)
        : nullptr;
}

/*-------------------------------------
    Get an object from the data buffer.
-------------------------------------*/
template <typename... data_t>
template <typename arg_t, typename... args_t> inline
void* tuple_t<data_t...>::getObjectAtIndex(unsigned index, unsigned offset, arg_t*, args_t*...) {
    return index == 0
        ? (void*)(dataBuffer+offset)
        : getObjectAtIndex<args_t...>(index-1, sizeof(arg_t)+offset, ((args_t*)nullptr)...);
}

/*-------------------------------------
    Get an object contained within *this. This object is specified using
    an array-like index.
-------------------------------------*/
template <typename... data_t> inline
void* tuple_t<data_t...>::getObject(unsigned index) {
    return getObjectAtIndex<data_t...>(index, 0, ((data_t*)nullptr)...);
}

/*-------------------------------------
    Non-retrieval methods
-------------------------------------*/
/*-------------------------------------
    Retrieve the number of objects contained within an tuple_t.
-------------------------------------*/
template <typename... data_t> constexpr
unsigned tuple_t<data_t...>::getNumObjects() const {
    return sizeof...(data_t);
}

} // end utils namespace
} // end ls namespace
