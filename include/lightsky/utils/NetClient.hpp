
#ifndef LS_UTILS_NETWORK_CLIENT_HPP
#define LS_UTILS_NETWORK_CLIENT_HPP

#include "lightsky/utils/NetNode.hpp"
#include "lightsky/utils/NetConnection.hpp"



struct _ENetHost;



namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * UDP Networking Client
-----------------------------------------------------------------------------*/
class NetClient final : public NetNode
{
  private:
    NetConnection mNode;

  public:
    ~NetClient() noexcept;

    NetClient() noexcept;

    NetClient(const NetClient&) = delete;

    NetClient(NetClient&&) noexcept;

    NetClient& operator=(const NetClient&) = delete;

    NetClient& operator=(NetClient&&) noexcept;

    int connect(uint32_t ip, uint16_t port, uint8_t maxChannels = 0, uint32_t inBps = 0, uint32_t outBps = 0, uint32_t timeoutMillis = 5000) noexcept;

    void disconnect() noexcept override;

    bool send(const void* pData, size_t numBytes, uint8_t channelId) noexcept;

    NetEventInfo poll(NetEvent* pNetInfo, uint32_t timeoutMillis) noexcept override;

    uint32_t server_host() const noexcept;

    uint16_t server_port() const noexcept;
};



/*-------------------------------------
 * Send a message to the server
-------------------------------------*/
inline bool NetClient::send(const void* pData, size_t numBytes, uint8_t channelId) noexcept
{
    return mNode.send(pData, numBytes, channelId);
}



/*-------------------------------------
 * Get the address of the server
-------------------------------------*/
inline uint32_t NetClient::server_host() const noexcept
{
    return mNode.address();
}



/*-------------------------------------
 * Get the server's connected port
-------------------------------------*/
inline uint16_t NetClient::server_port() const noexcept
{
    return mNode.port();
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_NETWORK_CLIENT_HPP */
