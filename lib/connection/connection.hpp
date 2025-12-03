#ifndef HAWAII_CONNECTION_H
#define HAWAII_CONNECTION_H

#include <Ethernet.h>
#include <EthernetUdp.h>

namespace hawaii::connection
{
    using MacAddress = uint8_t[6];
    using Port = uint16_t;

    struct Config
    {
        MacAddress controller_mac;
        IPAddress controller_address;
        Port controller_port;
        uint8_t controller_id;
        IPAddress server_address;
        Port server_hits_port;
        Port server_statistics_port;
    };

    struct System
    {
        EthernetClient ethernet;
        EthernetUDP udp;
    };

    enum struct CommandType : uint8_t
    {
        SetColor = 1,
        Reboot = 2,
        ToggleDevMode = 3
    };

    struct SetColorCommand
    {
        uint32_t color;
        uint32_t duration_ms;
        uint8_t controller_id;
    };

    struct RebootCommand
    {
        uint8_t controller_id;
    };

    union CommandPayload
    {
        SetColorCommand set_color;
        RebootCommand reboot;
    };

    struct Command
    {
        CommandType type;
        CommandPayload payload;
    };

    static_assert(sizeof(Command) <= UDP_TX_PACKET_MAX_SIZE, "Command size is too large");

    enum struct Event : uint8_t
    {
        Pinged = 1,
        Accelerated = 2,
        Restarted = 3,
    };

    auto init(System &connection, Config &config) -> void;
    [[nodiscard]] auto get_message(System &connection, Command &command) -> bool;
    auto send_ping(System &connection, Config const& config, unsigned long long sent_hit_packets) -> void;
    auto send_restarted(System &connection, Config const& config) -> void;
    auto send_acceleration(System &connection, Config const& config, float acceleration) -> void;
}

#endif
