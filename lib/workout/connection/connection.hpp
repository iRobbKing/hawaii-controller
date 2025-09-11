#ifndef HAWAII_CONNECTION_H
#define HAWAII_CONNECTION_H

#include <Ethernet.h>
#include <PubSubClient.h>

namespace hawaii::workout::connection
{
    using MacAddress = uint8_t[6];
    using Port = uint16_t;

    using Topic = char const*;
    using Payload = char const*;

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
        IPAddress controller_ip;
        IPAddress mqtt_server_address;
        Port mqtt_server_port;
        char const *mqtt_client_id;
        char const *mqtt_username;
        char const *mqtt_password;
    };

    struct System
    {
        EthernetClient ethernet;
        PubSubClient mqtt;
    };

    [[nodiscard]] auto init(System &connection, Config &config) -> Error;
    [[nodiscard]] auto reconnect(System &connection, Config const& config) -> bool;
    [[nodiscard]] auto try_get_setcolor(uint32_t& out_color) -> bool;
    auto try_get_dev_mode(bool& out_is_enabled) -> void;
    auto try_get_show_me(bool& out_is_enabled) -> void;
    [[nodiscard]] auto send_ping(System &connection, Config const& config) -> Error;
    [[nodiscard]] auto send_acceleration(System &connection, Config const& config, float acceleration) -> Error;
}

#endif
