
#include <cstring> // std::memset
#include <memory> // std::nothrow
#include <mutex>
#include <utility> // std::move()

#include <enet/enet.h>

#include "lightsky/utils/NetNode.hpp"



/*-----------------------------------------------------------------------------
 * Network Node Class
-----------------------------------------------------------------------------*/
namespace ls
{
namespace utils
{

/*-------------------------------------
 * Global UDP initialization
-------------------------------------*/
bool NetNode::network_init() noexcept
{
    static std::mutex G_MTX_NET_INIT;
    static bool G_NET_INITIALIZED = false;

    G_MTX_NET_INIT.lock();
    if (!G_NET_INITIALIZED)
    {
        if (enet_initialize() != 0)
        {
            //std::cerr << "Error: Unable to initialize enet." << std::endl;
            G_NET_INITIALIZED = false;
        }
        else
        {
            std::atexit(enet_deinitialize);
        }

        G_NET_INITIALIZED = true;
    }

    G_MTX_NET_INIT.unlock();

    return G_NET_INITIALIZED;
}



/*-------------------------------------
 * Parse the IP Address of a hostname
-------------------------------------*/
uint32_t NetNode::parse_hostname_to_ip(const char* pHostname) noexcept
{
    ENetAddress addr;
    if (enet_address_set_host(&addr, pHostname))
    {
        return 0;
    }

    return addr.host;
}



/*-------------------------------------
 * Resolve the IP of a host
-------------------------------------*/
uint32_t NetNode::resolve_hostname_to_ip(const char* pHostname) noexcept
{
    ENetAddress addr;
    if (enet_address_set_host(&addr, pHostname))
    {
        return 0;
    }

    return addr.host;
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
NetNode::NetNode() noexcept :
    mHost{nullptr},
    mLastEvent{nullptr}
{
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
NetNode::NetNode(NetNode&& server) noexcept :
    mHost{server.mHost},
    mLastEvent{server.mLastEvent}
{
    server.mHost = nullptr;
    server.mLastEvent = nullptr;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
NetNode& NetNode::operator=(NetNode&& server) noexcept
{
    mHost = server.mHost;
    server.mHost = nullptr;

    mLastEvent = server.mLastEvent;
    server.mLastEvent = nullptr;

    return *this;
}



/*-------------------------------------
 * Clean old data
-------------------------------------*/
void NetNode::clean_last_event_packet(_ENetEvent* pEvt) noexcept
{
    if (pEvt)
    {
        if (pEvt->packet)
        {
            enet_packet_destroy(pEvt->packet);
            pEvt->packet = nullptr;
        }

        if (pEvt->peer && pEvt->peer->data)
        {
            pEvt->peer->data = nullptr;
        }
    }
}



/*-------------------------------------
 * Initialize the event loop
-------------------------------------*/
bool NetNode::init_event_data() noexcept
{
    mLastEvent = new(std::nothrow) ENetEvent;
    if (!mLastEvent)
    {
        //std::cerr << "Could not allocate memory for a network event loop." << std::endl;
        return false;
    }

    std::memset(mLastEvent, 0, sizeof(struct _ENetEvent));
    return true;
}



/*-------------------------------------
 * Check if the connection is valid
-------------------------------------*/
bool NetNode::valid() const noexcept
{
    return mHost != nullptr;
}



/*-------------------------------------
 * Disconnect
-------------------------------------*/
void NetNode::disconnect() noexcept
{
    clean_last_event_packet(mLastEvent);

    delete mLastEvent;
    mLastEvent = nullptr;

    enet_host_destroy(mHost);
    mHost = nullptr;
}



/*-------------------------------------
 * Get the outgoing BPS
-------------------------------------*/
uint32_t NetNode::bytes_per_sec_outgoing() const noexcept
{
    return mHost->outgoingBandwidth;
}



/*-------------------------------------
 * Throttle the outgoing connection
-------------------------------------*/
void NetNode::bytes_per_sec_outgoing(uint32_t bps) noexcept
{
    enet_host_bandwidth_limit(mHost, mHost->incomingBandwidth, bps);
}



/*-------------------------------------
 * Get the incoming BPS
-------------------------------------*/
uint32_t NetNode::bytes_per_sec_incoming() const noexcept
{
    return mHost->incomingBandwidth;
}



/*-------------------------------------
 * Throttle the incoming BPS
-------------------------------------*/
void NetNode::bytes_per_sec_incoming(uint32_t bps) const noexcept
{
    enet_host_bandwidth_limit(mHost, bps, mHost->outgoingBandwidth);
}



/*-------------------------------------
 * Flush all incoming/outgoing packets
-------------------------------------*/
void NetNode::flush() noexcept
{
    enet_host_flush(mHost);
}



/*-------------------------------------
 * Wait for an event
-------------------------------------*/
NetEventInfo NetNode::wait(NetEvent* pNetInfo, uint32_t timeoutMillis) noexcept
{
    NetEventInfo evt;

    do
    {
        evt = this->poll(pNetInfo, timeoutMillis);
    } while (evt == NetEventInfo::NET_EVT_NONE);

    return evt;
}



} // end utils namespace
} // end ls namespace
