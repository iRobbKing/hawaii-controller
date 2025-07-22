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
    [[nodiscard]] auto send_message(System &connection, Topic topic, Payload payload) -> Error;
}

#endif
