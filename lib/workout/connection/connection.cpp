#include "connection.hpp"

namespace
{
    namespace hwc = hawaii::workout::connection;

    [[nodiscard]] auto init_ethernet(hwc::System &connection, hwc::Config &config) -> bool
    {
        // bool result = Ethernet.begin(config.controller_mac, IPAddress{192, 168, 50, 10});
        // Serial.println(Ethernet.localIP());
        // return result;
        Ethernet.begin(config.controller_mac, IPAddress{192, 168, 50, 10});
        Serial.println(Ethernet.localIP());
        return true;
    }

    auto callback(char* topic, byte* payload, unsigned int length) -> void
    {
        unsigned long const now = millis();
        Serial.print(*(unsigned short*)payload);
        Serial.print(" time ");
        Serial.println(now);
        Serial.println(topic);
    }

    auto init_mqtt(hwc::System &connection, hwc::Config &config) -> bool
    {
        connection.mqtt.setClient(connection.ethernet);
        connection.mqtt.setServer(config.mqtt_server_address, config.mqtt_server_port);
        connection.mqtt.setCallback(callback);

        bool result = connection.mqtt.connect(config.mqtt_client_id);
        // return connection.mqtt.connect(config.mqtt_client_id, config.mqtt_username, config.mqtt_password);

        connection.mqtt.subscribe("lamp/setcolor");

        Serial.print("MQTT state: ");
        Serial.println(connection.mqtt.state());

        return result;
    }
}

namespace hawaii::workout::connection
{
    auto init(System &connection, Config &config) -> Error
    {
        Serial.println("11111111");
        if (!init_ethernet(connection, config)) return Error::FailedToGetIp;
        Serial.println("22222");
        if (!init_mqtt(connection, config)) return Error::FailedToInitMqtt;
        Serial.println("33333");

        return Error::None;
    }

    auto send_message(System &connection, Topic topic, Payload payload) -> Error
    {
        Serial.println(connection.mqtt.connected() ? "MQTT connected" : "MQTT not connected");
        if (connection.mqtt.publish(topic, payload)) return Error::None;
        return Error::FailedToSendMessage;
    }
}
