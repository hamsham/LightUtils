
#include <utility> // std::move()

#include <enet/enet.h>

#include "lightsky/utils/NetClient.hpp"



/*-----------------------------------------------------------------------------
 * Network Client class
-----------------------------------------------------------------------------*/
namespace ls
{
namespace utils
{

/*-------------------------------------
 * Destructor
-------------------------------------*/
NetClient::~NetClient() noexcept
{
    disconnect();
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
NetClient::NetClient() noexcept :
    NetNode{},
    mNode{}
{
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
NetClient::NetClient(NetClient&& server) noexcept :
    NetClient{}
{
    mNode = std::move(server.mNode);
    NetNode::operator=(std::move(server));
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
NetClient& NetClient::operator=(NetClient&& server) noexcept
{
    mNode = std::move(server.mNode);
    NetNode::operator=(std::move(server));

    return *this;
}



/*-------------------------------------
 * Connect to a UDP server
-------------------------------------*/
int NetClient::connect(uint32_t ip, uint16_t port, uint8_t maxChannels, uint32_t inBps, uint32_t outBps, uint32_t timeoutMillis) noexcept
{
    ENetHost* pClient;
    NetConnection node;

    if (!NetNode::network_init())
    {
        return -1;
    }

    if (mHost)
    {
        disconnect();
    }

    pClient = enet_host_create(nullptr, 1, (size_t)maxChannels, inBps, outBps);
    if (!pClient)
    {
        //std::cerr << "Error: Unable to create a server at address " << ip << ':' << port << '.' << std::endl;
        return -2;
    }

    if (node.connect(pClient, ip, port, true, maxChannels, timeoutMillis) != 0)
    {
        enet_host_destroy(pClient);
        return -3;
    }

    if (!init_event_data())
    {
        disconnect();
        return -4;
    }

    mHost = pClient;
    mNode = std::move(node);

    return 0;
}



/*-------------------------------------
 * Disconnect from the server
-------------------------------------*/
void NetClient::disconnect() noexcept
{
    if (!mHost)
    {
        return;
    }

    if (mNode.valid())
    {
        mNode.disconnect();
    }

    enet_host_flush(mHost);

    NetNode::disconnect();
}



/*-------------------------------------
 * Poll the server
-------------------------------------*/
NetEventInfo NetClient::poll(NetEvent* pNetEvent, uint32_t timeoutMillis) noexcept
{
    int eventCode;

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
                pNetEvent->info = NetEventInfo::NET_EVT_CLIENT_CONNECTED;
                pNetEvent->host = mLastEvent->peer->address.host;
                pNetEvent->port = mLastEvent->peer->address.port;
                pNetEvent->channelId = mLastEvent->channelID;
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                mLastEvent->peer->data = nullptr;

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
