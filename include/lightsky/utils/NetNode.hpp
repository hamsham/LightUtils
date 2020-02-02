
#ifndef LS_UTILS_NETWORK_NODE_HPP
#define LS_UTILS_NETWORK_NODE_HPP

#include <cstdlib> // size_t
#include <cstdint>

#include "lightsky/utils/NetEvent.hpp"



struct _ENetHost;
struct _ENetEvent;



namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * Base class for networking clients and servers.
-----------------------------------------------------------------------------*/
class NetNode
{
  protected:
    struct _ENetHost* mHost;

    struct _ENetEvent* mLastEvent;

    static bool network_init() noexcept;

    static void clean_last_event_packet(_ENetEvent* pEvt) noexcept;

    bool init_event_data() noexcept;

  public:
    static uint32_t parse_hostname_to_ip(const char* pHostname) noexcept;

    static uint32_t resolve_hostname_to_ip(const char* pHostname) noexcept;

    virtual ~NetNode() noexcept;

    NetNode() noexcept;

    NetNode(const NetNode&) = delete;

    NetNode(NetNode&&) noexcept;

    NetNode& operator=(const NetNode&) = delete;

    NetNode& operator=(NetNode&&) noexcept;

    bool valid() const noexcept;

    virtual void disconnect() noexcept;

    uint32_t bytes_per_sec_outgoing() const noexcept;

    void bytes_per_sec_outgoing(uint32_t bps) noexcept;

    uint32_t bytes_per_sec_incoming() const noexcept;

    void bytes_per_sec_incoming(uint32_t bps) const noexcept;

    void flush() noexcept;

    NetEventInfo poll(NetEvent* pNetInfo) noexcept;

    virtual NetEventInfo poll(NetEvent* pNetInfo, uint32_t timeoutMillis) noexcept = 0;

    NetEventInfo wait(NetEvent* pNetInfo) noexcept;

    virtual NetEventInfo wait(NetEvent* pNetInfo, uint32_t timeoutMillis) noexcept;
};



/*-------------------------------------
 * Poll for an event
-------------------------------------*/
inline NetEventInfo NetNode::poll(NetEvent* pNetInfo) noexcept
{
    return this->poll(pNetInfo, 0);
}



/*-------------------------------------
 * Wait for an event
-------------------------------------*/
inline NetEventInfo NetNode::wait(NetEvent* pNetInfo) noexcept
{
    return this->wait(pNetInfo, ~(uint32_t)0u);
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_NETWORK_NODE_HPP */
