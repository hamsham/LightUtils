
#include <enet/enet.h>

#include "lightsky/utils/NetConnection.hpp"



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
namespace ls
{
namespace utils
{

/*-------------------------------------
 * Constructor
-------------------------------------*/
NetConnection::NetConnection() noexcept :
    mPeer{nullptr}
{
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
NetConnection::NetConnection(NetConnection&& server) noexcept :
    mPeer{server.mPeer}
{
    server.mPeer = nullptr;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
NetConnection& NetConnection::operator=(NetConnection&& server) noexcept
{
    mPeer = server.mPeer;
    server.mPeer = nullptr;

    return *this;
}



/*-------------------------------------
 * Determine if a connection has been made
-------------------------------------*/
bool NetConnection::valid() const noexcept
{
    return mPeer != nullptr;
}



/*-------------------------------------
 * Connect to a server
-------------------------------------*/
int NetConnection::connect(_ENetHost* pHost, uint32_t ipAddr, uint16_t port, bool clientConnection, uint8_t maxChannels, uint32_t timeoutMillis) noexcept
{
    if (mPeer)
    {
        disconnect();
    }

    if (!pHost || (!clientConnection && !pHost->address.host))
    {
        //std::cerr << "No host specified for network connection." << std::endl;
        return -1;
    }

    ENetAddress address;
    address.host = ipAddr;
    address.port = port;

    struct _ENetPeer* pPeer = enet_host_connect(pHost, &address, maxChannels, 0);

    if (!pPeer)
    {
        //std::cerr << "Could not connect to host at " << address.host << ':' << address.port << '.' << std::endl;
        return -2;
    }

    ENetEvent evt;
    if (enet_host_service(pHost, &evt, timeoutMillis) <= 0 || evt.type != ENET_EVENT_TYPE_CONNECT)
    {
        enet_peer_disconnect(pPeer, 0);
        //std::cerr << "Timeout while initiating network connection." << std::endl;
        return -3;
    }

    mPeer = pPeer;

    return 0;
}



/*-------------------------------------
 * Connect to a server using pre-created data.
-------------------------------------*/
int NetConnection::connect(_ENetPeer* pPeer) noexcept
{
    if (!pPeer || !pPeer->host)
    {
        return -1;
    }

    disconnect();

    mPeer = pPeer;

    return 0;
}



/*-------------------------------------
 * Disconnect from the server
-------------------------------------*/
void NetConnection::disconnect() noexcept
{
    if (mPeer)
    {
        enet_peer_disconnect(mPeer, 0);

        mPeer = nullptr;
    }
}



/*-------------------------------------
 * Send data to another peer
-------------------------------------*/
int NetConnection::send(const void* pData, size_t numBytes, uint8_t channelId) noexcept
{
    ENetPacket* packet = enet_packet_create(pData, numBytes, ENET_PACKET_FLAG_RELIABLE | ENET_PACKET_FLAG_NO_ALLOCATE);
    if (!packet)
    {
        return -1;
    }

    return enet_peer_send(mPeer, channelId, packet);
    //enet_packet_destroy(packet);
}



/*-------------------------------------
 * Get the address of the connection
-------------------------------------*/
uint32_t NetConnection::address() const noexcept
{
    return mPeer ? mPeer->address.host : 0;
}



/*-------------------------------------
 * Get the port of the connection.
-------------------------------------*/
uint16_t NetConnection::port() const noexcept
{
    return mPeer ? mPeer->address.port : 0;
}



} // end utils namespace
} // end ls namespace
