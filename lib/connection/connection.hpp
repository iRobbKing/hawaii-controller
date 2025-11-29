#ifndef HAWAII_CONNECTION_H
#define HAWAII_CONNECTION_H

#include <Ethernet.h>
#include <EthernetUdp.h>

namespace hawaii::connection
{
    using MacAddress = uint8_t[6];
    using Port = uint16_t;

    enum struct Error
    {
        None = 0,
        FailedToGetIp = 1,
        FailedToInitMqtt = 2,
        FailedToSendMessage = 3,
    };

    struct Config
    {
        MacAddress controller_mac;
        IPAddress server_address;
        Port server_port;
        Port local_port;
        uint8_t controller_id;
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
    };

    union CommandPayload
    {
        SetColorCommand set_color;
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
    };

    [[nodiscard]] auto init(System &connection, Config &config) -> Error;
    [[nodiscard]] auto loop(System &connection) -> void;
    [[nodiscard]] auto get_message(System &connection, Command &command) -> bool;
    auto send_ping(System &connection, Config const& config) -> void;
    auto send_acceleration(System &connection, Config const& config, float acceleration) -> void;
}

#endif
