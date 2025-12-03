#include "connection.hpp"

namespace hawaii::connection
{
    auto init(System &connection, Config &config) -> void
    {
        Ethernet.begin(config.controller_mac, config.controller_address);

        connection.udp.begin(config.controller_port);
    }

    auto get_message(System &connection, Command &message) -> bool
    {
        unsigned long constexpr message_size = sizeof(CommandType) + sizeof(CommandPayload);
        int packet_size = connection.udp.parsePacket();
        if (packet_size != message_size)
            return false;

        byte packet_buffer[message_size];
        int const read_bytes = connection.udp.read(packet_buffer, message_size);
        if (read_bytes != message_size)
            return false;

        switch (packet_buffer[0])
        {
            case static_cast<uint8_t>(CommandType::SetColor):
            case static_cast<uint8_t>(CommandType::Reboot):
            case static_cast<uint8_t>(CommandType::ToggleDevMode):
                break;
            default:
                return false;
        }

        memcpy(&message.type, packet_buffer, sizeof(CommandType));
        memcpy(&message.payload, packet_buffer + sizeof(CommandType), sizeof(CommandPayload));

        return true;
    }

    auto send_ping(System &connection, Config const& config, unsigned long long sent_hit_packets) -> void
    {
        connection.udp.beginPacket(config.server_address, config.server_statistics_port);
        connection.udp.write(static_cast<uint8_t>(Event::Pinged));
        connection.udp.write(config.controller_id);
        connection.udp.write((uint8_t*)&sent_hit_packets, 8);
        connection.udp.endPacket();
    }

    auto send_restarted(System &connection, Config const& config) -> void
    {
        connection.udp.beginPacket(config.server_address, config.server_statistics_port);
        connection.udp.write(static_cast<uint8_t>(Event::Restarted));
        connection.udp.write(config.controller_id);
        connection.udp.endPacket();
    }

    auto send_acceleration(System &connection, Config const& config, float const acceleration) -> void
    {
        connection.udp.beginPacket(config.server_address, config.server_hits_port);
        connection.udp.write(static_cast<uint8_t>(Event::Accelerated));
        connection.udp.write(config.controller_id);
        connection.udp.write(reinterpret_cast<const uint8_t*>(&acceleration), sizeof(acceleration));
        connection.udp.endPacket();
    }
}
