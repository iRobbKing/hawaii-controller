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
            case static_cast<uint8_t>(CommandType::StartFitboxingRound):
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
        uint8_t buffer[10] = {0};
        buffer[0] = static_cast<uint8_t>(Event::Pinged);
        buffer[1] = config.controller_id;
        memcpy(&buffer[2], &sent_hit_packets, 8);

        connection.udp.beginPacket(config.server_address, config.server_statistics_port);
        connection.udp.write(buffer, sizeof(buffer));
        connection.udp.endPacket();
    }

    auto send_restarted(System &connection, Config const& config) -> void
    {
        uint8_t buffer[2] = {0};
        buffer[0] = static_cast<uint8_t>(Event::Restarted);
        buffer[1] = config.controller_id;

        connection.udp.beginPacket(config.server_address, config.server_statistics_port);
        connection.udp.write(buffer, sizeof(buffer));
        connection.udp.endPacket();
    }

    auto send_acceleration(System &connection, Config const& config, float const acceleration, uint32_t const tact_round_index) -> void
    {
        uint8_t buffer[10] = {0};
        buffer[0] = static_cast<uint8_t>(Event::Accelerated);
        buffer[1] = config.controller_id;
        memcpy(&buffer[2], &acceleration, sizeof(acceleration));
        memcpy(&buffer[6], &tact_round_index, sizeof(tact_round_index));
        
        connection.udp.beginPacket(config.server_address, config.server_hits_port);
        connection.udp.write(buffer, sizeof(buffer));
        connection.udp.endPacket();
    }
}
