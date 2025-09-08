#include "connection.hpp"

namespace
{
    namespace hwc = hawaii::workout::connection;

    [[nodiscard]] auto init_ethernet(hwc::System &connection, hwc::Config &config) -> bool
    {
        return Ethernet.begin(config.controller_mac);
        // Ethernet.begin(config.controller_mac, config.controller_ip);
        // Serial.println(Ethernet.localIP());
        // return true;
    }

    byte mac_address[6] = { 0, 0, 0, 0, 0, 0 };
    bool has_received_setcolor = false;
    uint32_t received_setcolor_color = 0;
    bool has_received_dev_mode = false;
    bool received_dev_mode = false;

    auto callback(char* topic, byte* payload, unsigned int length) -> void
    {
        if (!strcmp(topic, "d"))
        {
            has_received_dev_mode = true;
            received_dev_mode = *((bool*)payload);
            return;    
        }

        if (length != sizeof(mac_address) + sizeof(received_setcolor_color))
            return;

        if (payload[4] != 254 && mac_address[4] != payload[4] || payload[5] != 170 && mac_address[5] != payload[5])
            return;

        if (!strcmp(topic, "l"))
        {
            has_received_setcolor = true;
            received_setcolor_color = *((uint32_t*)(payload + sizeof(mac_address)));
        } 
    }

    auto init_mqtt(hwc::System &connection, hwc::Config &config) -> bool
    {
        connection.mqtt.setClient(connection.ethernet);
        connection.mqtt.setServer(config.mqtt_server_address, config.mqtt_server_port);
        connection.mqtt.setCallback(callback);

        bool result = connection.mqtt.connect(config.mqtt_client_id);

        connection.mqtt.subscribe("l");
        connection.mqtt.subscribe("d");

        return result;
    }
}

namespace hawaii::workout::connection
{
    auto init(System &connection, Config &config) -> Error
    {
        memcpy(mac_address, config.controller_mac, sizeof(mac_address));

        if (!init_ethernet(connection, config)) return Error::FailedToGetIp;;
        if (!init_mqtt(connection, config)) return Error::FailedToInitMqtt;

        return Error::None;
    }

    auto try_get_setcolor(uint32_t& out_color) -> bool
    {
        if (has_received_setcolor)
        {
            has_received_setcolor = false;
            out_color = received_setcolor_color;
            return true;
        }

        return false;
    }

    auto try_get_dev_mode(bool& out_is_enabled) -> void
    {
        if (has_received_dev_mode)
        {
            has_received_dev_mode = false;
            out_is_enabled = received_dev_mode;
        }
    }

    auto send_ping(System &connection, Config const& config) -> Error
    {
        Topic constexpr topic = "controller/ping";

        if (connection.mqtt.publish(topic, (Payload)config.controller_mac)) return Error::None;
        return Error::FailedToSendMessage;
    }

    auto send_acceleration(System &connection, Config const& config, float const acceleration) -> Error
    {
        Topic constexpr topic = "accelerator/acceleration";

        uint8_t payload[sizeof(config.controller_mac) + sizeof(acceleration)];
        memcpy(payload, config.controller_mac, sizeof(config.controller_mac));
        memcpy(payload + sizeof(config.controller_mac), &acceleration, sizeof(acceleration));

        if (connection.mqtt.publish(topic, (Payload)payload)) return Error::None;
        return Error::FailedToSendMessage;
    }
}
