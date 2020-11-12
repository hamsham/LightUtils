
#ifndef LS_UTILS_NETWORK_CONNECTION_HPP
#define LS_UTILS_NETWORK_CONNECTION_HPP

#include <cstdint>

struct _ENetHost;
struct _ENetPeer;



namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Basic UDP Connection
-----------------------------------------------------------------------------*/
class NetConnection
{
  public:
    enum : uint32_t
    {
        DEFAULT_CONNECTION_TIMEOUT_MS = 5000, // 5 seconds
        CHANNEL_COUNT_LIMITLESS = 0,
        CONNECTION_BYTES_PER_SEC_LIMITLESS = 0,
    };

  private:
    struct _ENetPeer* mPeer;

  public:
    ~NetConnection() noexcept = default;

    NetConnection() noexcept;

    NetConnection(const NetConnection&) = delete;

    NetConnection(NetConnection&&) noexcept;

    NetConnection& operator=(const NetConnection&) = delete;

    NetConnection& operator=(NetConnection&&) noexcept;

    bool valid() const noexcept;

    int connect(
        _ENetHost* pHost,
        uint32_t ipAddr,
        uint16_t port,
        bool clientConnection,
        uint8_t maxChannels = (uint32_t)CHANNEL_COUNT_LIMITLESS,
        uint32_t timeoutMillis = (uint32_t)DEFAULT_CONNECTION_TIMEOUT_MS
    ) noexcept;

    int connect(_ENetPeer* pHost) noexcept;

    void disconnect() noexcept;

    int send(const void* pData, size_t numBytes, uint8_t channelId) noexcept;

    uint32_t address() const noexcept;

    uint16_t port() const noexcept;

    const struct _ENetPeer* native_handle() const noexcept;

    struct _ENetPeer* native_handle() noexcept;
};



/*-------------------------------------
 * Native handle to the underlying ENet data (const)
-------------------------------------*/
inline const struct _ENetPeer* NetConnection::native_handle() const noexcept
{
    return mPeer;
}



/*-------------------------------------
 * Native handle to the underlying ENet data
-------------------------------------*/
inline struct _ENetPeer* NetConnection::native_handle() noexcept
{
    return mPeer;
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_NETWORK_CONNECTION_HPP */
