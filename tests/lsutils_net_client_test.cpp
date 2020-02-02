
#include <cstdlib> // std::atexit
#include <iostream>

#include "lightsky/utils/NetClient.hpp"
#include "lsutils_net_test.hpp"

using namespace ls::utils;



int main()
{
    NetClient client;
    NetEvent evt;
    int packetCount = 2;

    if (client.connect(NetNode::resolve_hostname_to_ip(TEST_ADDRESS), TEST_PORT, 2) != 0)
    {
        std::cerr << "Failed to connect to the server." << std::endl;
        return -1;
    }
    else
    {
        std::cout
            << "Successfully connected to the remote server!"
            << "\n\tHost: " << client.server_host()
            << "\n\tPort: " << client.server_port()
            << "\n\tIncoming BPS: " << client.bytes_per_sec_incoming()
            << "\n\tOutgoing BPS: " << client.bytes_per_sec_outgoing()
            << std::endl;

        if (client.send(TEST_PACKET, TEST_PACKET_SIZE, 0) != 0)
        {
            std::cerr << "Error: Unable to send data to the server" << std::endl;
            return -2;
        }

        client.flush();
    }

    while (client.valid())
    {
        switch (client.wait(&evt))
        {
            case NET_EVT_NONE:
                break;

            case NET_EVT_CLIENT_DATA:
                std::cout
                    << "Server data received!"
                    << "\n\tData Size:   " << evt.numBytes
                    << "\n\tPacket Data: " << reinterpret_cast<const char*>(evt.pData)
                    << "\n\tChannel ID:  " << (unsigned)evt.channelId
                    << std::endl;
                --packetCount;
                break;

            case NET_EVT_CLIENT_CONNECTED:
                std::cout << "Successfully connected to the server." << std::endl;
                break;

            case NET_EVT_CLIENT_DISCONNECTED:
                std::cout << "Server forcefully terminated connection." << std::endl;
                client.disconnect();
                break;

            default:
                break;
        }

        if (!packetCount)
        {
            client.disconnect();
        }
    }

    return 0;
}
