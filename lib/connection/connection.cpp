#include "connection.hpp"

namespace
{
    namespace hc = hawaii::connection;

    [[nodiscard]] auto init_ethernet(hc::System &connection, hc::Config &config) -> bool
    {
        return Ethernet.begin(config.controller_mac);
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
            memcpy(&received_dev_mode, payload, sizeof(received_dev_mode));
            return;    
        }

        if ((length == sizeof(mac_address) + sizeof(received_setcolor_color)) && !strcmp(topic, "l"))
        {
            has_received_setcolor = true;
            memcpy(&received_setcolor_color, payload + sizeof(mac_address), sizeof(received_setcolor_color));
        } 
    }

    auto init_mqtt(hc::System &connection, hc::Config &config) -> bool
    {
        connection.mqtt.setClient(connection.ethernet);
        connection.mqtt.setServer(config.mqtt_server_address, config.mqtt_server_port);
        connection.mqtt.setCallback(callback);

        if (connection.mqtt.connect(config.mqtt_client_id))
        {
            connection.mqtt.subscribe("l");
            connection.mqtt.subscribe("d");
        }

        return connection.mqtt.connected();
    }
}

namespace hawaii::connection
{
    auto init(System &connection, Config &config) -> Error
    {
        if (!init_ethernet(connection, config))
            return Error::FailedToGetIp;

        if (!init_mqtt(connection, config))
            return Error::FailedToInitMqtt;

        return Error::None;
    }

    auto loop(System &connection, Config const& config, unsigned long const now) -> bool
    {
        if (connection.mqtt.connected())
        {
            connection.mqtt.loop();
            return true;
        }
        else
        {
            static unsigned long last_reconnect_time = 0;
            if (5000 < now - last_reconnect_time)
            {
                last_reconnect_time = now;

                if (connection.mqtt.connect(config.mqtt_client_id))
                {
                    connection.mqtt.subscribe("l");
                    connection.mqtt.subscribe("d");
                }

                if (connection.mqtt.connected())
                {
                    last_reconnect_time = 0;
                    return true;
                }
            }

            return false;
        }
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

    auto send_ping(System &connection, Config const& config) -> void
    {
        Topic constexpr topic = "controller/ping";

        char payload[sizeof(config.controller_mac) + 1];
        memcpy(payload, config.controller_mac, sizeof(config.controller_mac));
        payload[sizeof(config.controller_mac)] = '\0';

        connection.mqtt.publish(topic, payload);
    }

    auto send_acceleration(System &connection, Config const& config, float const acceleration) -> void
    {
        Topic constexpr topic = "accelerator/acceleration";

        char payload[sizeof(config.controller_mac) + sizeof(acceleration) + 1];
        memcpy(payload, config.controller_mac, sizeof(config.controller_mac));
        memcpy(payload + sizeof(config.controller_mac), &acceleration, sizeof(acceleration));
        payload[sizeof(config.controller_mac) + sizeof(acceleration)] = '\0';
        
        connection.mqtt.publish(topic, payload);
    }
}
