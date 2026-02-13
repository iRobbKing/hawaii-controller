#include "connection.hpp"

namespace
{
    namespace hwc = hawaii::workout::connection;

    auto setup_ethernet(hwc::Config &config) -> void
    {
        pinMode(53, OUTPUT);
        pinMode(4, OUTPUT);
        digitalWrite(4, HIGH); 

        Ethernet.begin(config.controller_mac, config.controller_ip);
        Ethernet.setRetransmissionTimeout(200);
        Ethernet.setRetransmissionCount(3);
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
}

namespace hawaii::workout::connection
{
    auto init(System &connection, Config &config) -> Error
    {
        setup_ethernet(config);

        if (Ethernet.hardwareStatus() == EthernetNoHardware)
            return Error::FailedToGetIp;

        connection.mqtt.setClient(connection.ethernet);
        connection.mqtt.setServer(config.mqtt_server_address, config.mqtt_server_port);
        connection.mqtt.setCallback(callback);
        connection.mqtt.setSocketTimeout(4);
        connection.mqtt.setKeepAlive(5);

        if (connection.mqtt.connect(config.mqtt_client_id))
        {
            connection.mqtt.subscribe("l");
            connection.mqtt.subscribe("d");
        }

        return Error::None;
    }

    auto loop(System &connection, Config &config, unsigned long const now) -> bool
    {
        if (connection.mqtt.connected())
        {
            connection.mqtt.loop();
            return true;
        }

        static unsigned long last_reconnect_time = 0;
        static uint8_t failed_reconnects = 0;

        if (5000 < now - last_reconnect_time)
        {
            last_reconnect_time = now;

            if (failed_reconnects >= 3)
            {
                failed_reconnects = 0;
                setup_ethernet(config);
            }

            if (connection.mqtt.connect(config.mqtt_client_id))
            {
                connection.mqtt.subscribe("l");
                connection.mqtt.subscribe("d");
                failed_reconnects = 0;
                last_reconnect_time = 0;
            }
            else
            {
                failed_reconnects++;
            }
        }

        return connection.mqtt.connected();
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

        if (!connection.mqtt.publish(topic, payload))
            connection.mqtt.disconnect();
    }

    auto send_acceleration(System &connection, Config const& config, float const acceleration) -> void
    {
        Topic constexpr topic = "accelerator/acceleration";

        char payload[sizeof(config.controller_mac) + sizeof(acceleration) + 1];
        memcpy(payload, config.controller_mac, sizeof(config.controller_mac));
        memcpy(payload + sizeof(config.controller_mac), &acceleration, sizeof(acceleration));
        payload[sizeof(config.controller_mac) + sizeof(acceleration)] = '\0';

        if (!connection.mqtt.publish(topic, payload))
            connection.mqtt.disconnect();
    }
}
