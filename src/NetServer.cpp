
//#include <iostream>
#include <memory> // std::nothrow
#include <utility> // std::move()

#include <enet/enet.h>

#include "lightsky/utils/NetServer.hpp"



/*-----------------------------------------------------------------------------
 * UDP Server Class
-----------------------------------------------------------------------------*/
namespace ls
{
namespace utils
{

/*-------------------------------------
 * Destructor
-------------------------------------*/
NetServer::~NetServer() noexcept
{
    disconnect();
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
NetServer::NetServer() noexcept :
    NetNode{},
    mClients{}
{
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
NetServer::NetServer(NetServer&& server) noexcept :
    NetNode{std::move(server)},
    mClients{std::move(server.mClients)}
{
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
NetServer& NetServer::operator=(NetServer&& server) noexcept
{

    mClients = std::move(server.mClients);

    return *this;
}



/*-------------------------------------
 * Initialize the server
-------------------------------------*/
int NetServer::connect(uint32_t ip, uint16_t port, size_t maxClients, uint8_t maxChannels, uint32_t inBps, uint32_t outBps) noexcept
{
    ENetAddress address;
    ENetHost* pServer;

    if (!NetNode::network_init())
    {
        return -1;
    }

    if (mHost)
    {
        disconnect();
    }

    address.host = ip;
    address.port = port;

    pServer = enet_host_create(&address, maxClients, (size_t)maxChannels, inBps, outBps);
    if (!pServer)
    {
        //std::cerr << "Error: Unable to create a server at address " << ip << ':' << port << '.' << std::endl;
        return -2;
    }
    else
    {
        //std::cout
        //    << "Successfully initialized a server"
        //    << "\n\tAddress: " << address.host
        //    << "\n\tPort:    " << address.port
        //    << std::endl;
        mHost = pServer;
    }

    if (!init_event_data())
    {
        disconnect();
        return -3;
    }

    return 0;
}



/*-------------------------------------
 * Terminate the server
-------------------------------------*/
void NetServer::disconnect() noexcept
{
    if (!mHost)
    {
        return;
    }

    for (NetConnection& node : mClients)
    {
        node.disconnect();
    }

    mClients.clear();

    enet_host_flush(mHost);

    NetNode::disconnect();
}



/*-------------------------------------
 * Terminate a client
-------------------------------------*/
void NetServer::disconnect(uint32_t clientIp) noexcept
{
    for (size_t i = 0; i < mClients.size(); ++i)
    {
        if (mClients[i].address() == clientIp)
        {
            mClients[i].disconnect();

            enet_host_flush(mHost);

            mClients.erase(mClients.begin()+i);
            break;
        }
    }
}



/*-------------------------------------
 * Broadcast to all peers
-------------------------------------*/
int NetServer::broadcast(const void* pData, size_t numBytes, uint8_t channelId) noexcept
{
    ENetPacket* packet = enet_packet_create(pData, numBytes, ENET_PACKET_FLAG_RELIABLE | ENET_PACKET_FLAG_NO_ALLOCATE);
    if (!packet)
    {
        return -1;
    }

    enet_host_broadcast(mHost, channelId, packet);
    //enet_packet_destroy(packet);

    return 0;
}



/*-------------------------------------
 * Poll the server for events.
-------------------------------------*/
NetEventInfo NetServer::poll(NetEvent* pNetEvent, uint32_t timeoutMillis) noexcept
{
    int eventCode;
    size_t i;

    pNetEvent->info = NetEventInfo::NET_EVT_NONE;
    pNetEvent->host = 0;
    pNetEvent->port = 0;
    pNetEvent->channelId = 0xFF;
    pNetEvent->pData = nullptr;
    pNetEvent->numBytes = 0;

    clean_last_event_packet(mLastEvent);

    eventCode = enet_host_service(mHost, mLastEvent, timeoutMillis);

    if (eventCode > 0)
    {
        switch (mLastEvent->type)
        {
            case ENET_EVENT_TYPE_NONE:
                break;

            case ENET_EVENT_TYPE_CONNECT:
                mClients.emplace_back(NetConnection{});
                mClients.back().connect(mLastEvent->peer);

                pNetEvent->info = NetEventInfo::NET_EVT_CLIENT_CONNECTED;
                pNetEvent->host = mLastEvent->peer->address.host;
                pNetEvent->port = mLastEvent->peer->address.port;
                pNetEvent->channelId = mLastEvent->channelID;
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                mLastEvent->peer->data = nullptr;

                for (i = 0; i < mClients.size(); ++i)
                {
                    if (mClients[i].address() == mLastEvent->peer->address.host)
                    {
                        mClients.erase(mClients.begin()+i);
                        break;
                    }
                }

                pNetEvent->info = NetEventInfo::NET_EVT_CLIENT_DISCONNECTED;
                pNetEvent->host = mLastEvent->peer->address.host;
                pNetEvent->port = mLastEvent->peer->address.port;
                pNetEvent->channelId = mLastEvent->channelID;
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                pNetEvent->info = NetEventInfo::NET_EVT_CLIENT_DATA;
                pNetEvent->host = mLastEvent->peer->address.host;
                pNetEvent->port = mLastEvent->peer->address.port;
                pNetEvent->channelId = mLastEvent->channelID;
                pNetEvent->pData = mLastEvent->packet->data;
                pNetEvent->numBytes = mLastEvent->packet->dataLength;
                break;

            default:
                pNetEvent->info = NetEventInfo::NET_EVT_CLIENT_TIMEOUT;
                break;
        }
    }
    else if (eventCode < 0)
    {
        pNetEvent->info = NetEventInfo::NET_EVT_ERROR;
    }

    return pNetEvent->info;
}



} // end utils namespace
} // end ls namespace
