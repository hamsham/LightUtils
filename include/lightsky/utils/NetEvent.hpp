
#ifndef LS_UTILS_NETWORK_EVENT_HPP
#define LS_UTILS_NETWORK_EVENT_HPP

#include <cstdint>
#include <cstdlib> // size_t



namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Event notification types
-----------------------------------------------------------------------------*/
enum NetEventInfo : uint32_t
{
    NET_EVT_CLIENT_CONNECTED,
    NET_EVT_CLIENT_DISCONNECTED,
    NET_EVT_CLIENT_DATA,
    NET_EVT_CLIENT_TIMEOUT,
    NET_EVT_ERROR,
    NET_EVT_NONE,
};



/*-----------------------------------------------------------------------------
 * Networking Event Data
-----------------------------------------------------------------------------*/
struct NetEvent
{
    NetEventInfo info;

    uint32_t host;

    uint16_t port;

    uint8_t channelId;

    const uint8_t* pData;

    size_t numBytes;
};



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_NETWORK_EVENT_HPP */
