
#include <cstdlib> // std::atexit
#include <iostream>

#include "lightsky/utils/NetServer.hpp"
#include "lsutils_net_test.hpp"

using namespace ls::utils;



int main()
{
    size_t numConnections = 3;
    NetServer server;
    NetEvent evt;

    if (server.connect(0, TEST_PORT, numConnections) != 0)
    {
        std::cerr << "Error: Unable to initialize enet." << std::endl;
        return -1;
    }
    else
    {
        server.bytes_per_sec_incoming(0);
        server.bytes_per_sec_outgoing(0);

        std::cout
            << "Successfully initialized the server:"
            << "\n\tIncoming BPS: " << server.bytes_per_sec_incoming()
            << "\n\tOutgoing BPS: " << server.bytes_per_sec_outgoing()
            << std::endl;
    }

    while (numConnections)
    {
        const NetEventInfo eventCode = server.wait(&evt);

        switch (eventCode)
        {
            case NetEventInfo::NET_EVT_NONE:
                break;

            case NetEventInfo::NET_EVT_CLIENT_CONNECTED:
                std::cout
                    << "Client connected!"
                    << "\n\tAddress: " << evt.host
                    << "\n\tPort:    " << evt.port
                    << std::endl;
                server.send(server.clients().size()-1, "Hi Buddy!", 10, 1);
                break;

            case NetEventInfo::NET_EVT_CLIENT_DISCONNECTED:
                std::cout
                    << "Client disconnected!"
                    << "\n\tAddress: " << evt.host
                    << "\n\tPort:    " << evt.port
                    << std::endl;
                break;

            case NetEventInfo::NET_EVT_CLIENT_DATA:
                std::cout
                    << "Client data received!"
                    << "\n\tData Size:   " << evt.numBytes
                    << "\n\tPacket Data: " << reinterpret_cast<const char*>(evt.pData)
                    << "\n\tChannel ID:  " << (unsigned)evt.channelId
                    << std::endl;
                --numConnections;
                if (!numConnections)
                {
                    server.broadcast(TEST_RESPONSE, TEST_RESPONSE_SIZE, 1);
                    server.flush();
                }

            default:
                break;
        }
    }

    server.disconnect();

    return 0;
}
