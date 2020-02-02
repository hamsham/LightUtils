
#ifndef ENET_TEST_HPP
#define ENET_TEST_HPP

#include <cstdint>
#include <cstdlib> // size_t


static constexpr char TEST_ADDRESS[] = "localhost";
static constexpr uint16_t TEST_PORT = 5500;

static constexpr char TEST_PACKET[] = "Hello World!";
static constexpr size_t TEST_PACKET_SIZE = sizeof(TEST_PACKET) * sizeof(char);

static constexpr char TEST_RESPONSE[] = "Hello from the server!";
static constexpr size_t TEST_RESPONSE_SIZE = sizeof(TEST_RESPONSE) * sizeof(char);



#endif // ENET_TEST_HPP
