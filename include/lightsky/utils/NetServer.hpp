
#ifndef LS_UTILS_NETWORK_SERVER_HPP
#define LS_UTILS_NETWORK_SERVER_HPP

#include <cstdint>
#include <vector>

#include "lightsky/utils/NetNode.hpp"
#include "lightsky/utils/NetConnection.hpp"



namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * UDP Server
-----------------------------------------------------------------------------*/
class NetServer final : public NetNode
{
  private:
    std::vector<NetConnection> mClients;

  public:
    ~NetServer() noexcept;

    NetServer() noexcept;

    NetServer(const NetServer&) = delete;

    NetServer(NetServer&&) noexcept;

    NetServer& operator=(const NetServer&) = delete;

    NetServer& operator=(NetServer&&) noexcept;

    int connect(uint32_t ip, uint16_t port, size_t maxClients, uint8_t maxChannels = 0, uint32_t inBps = 0, uint32_t outBps = 0) noexcept;

    void disconnect(uint32_t clientIp) noexcept;

    void disconnect() noexcept override;

    int broadcast(const void* pData, size_t numBytes, uint8_t channelId) noexcept;

    int send(size_t clientId, const void* pData, size_t numBytes, uint8_t channelId) noexcept;

    NetEventInfo poll(NetEvent* pNetInfo, uint32_t timeoutMillis) noexcept override;

    const std::vector<NetConnection>& clients() const noexcept;

    const NetConnection& client(size_t clientId) const noexcept;

    NetConnection& client(size_t clientId) noexcept;
};



/*-------------------------------------
 * Send data to a specific client
-------------------------------------*/
inline int NetServer::send(size_t clientId, const void* pData, size_t numBytes, uint8_t channelId) noexcept
{
    if (clientId >= mClients.size())
    {
        return -1;
    }

    return mClients[clientId].send(pData, numBytes, channelId);
}



/*-------------------------------------
 * Get the currently connected clients.
-------------------------------------*/
inline const std::vector<NetConnection>& NetServer::clients() const noexcept
{
    return mClients;
}



/*-------------------------------------
 * Get a client connection (const)
-------------------------------------*/
inline const NetConnection& NetServer::client(size_t clientId) const noexcept
{
    return mClients[clientId];
}



/*-------------------------------------
 * Get a client connection
-------------------------------------*/
inline NetConnection& NetServer::client(size_t clientId) noexcept
{
    return mClients[clientId];
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_NETWORK_SERVER_HPP */
