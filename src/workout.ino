#include <Arduino.h>

#include "workout.hpp"

namespace hw = hawaii::workout;
namespace ha = hawaii::accelerator;
namespace hc = hawaii::connection;
namespace hl = hawaii::lamp;

hw::Config config = 
{
    .accelerator = {
        .sensitivity = ACCEL_FS::A8G
    },
    .connection = {
        .controller_mac = {CONTROLLER_MAC},
        .controller_address = IPAddress{CONTROLLER_IP},
        .controller_port = 5000,
        .controller_id = CONTROLLER_ID,
        .server_address = IPAddress{SERVER_IP},
        .server_hits_port = 5111,
        .server_statistics_port = 5112,
    },
    .lamp = {
        .light_quantity = 24,
        .light_pin = 6,
        .light_brightness = 255
    }
};

hw::System workout{};
hw::State state{};

auto setup() -> void
{
    Wire.begin();
    Wire.setWireTimeout(25000, true);

    hw::init(workout, config, state);
}

auto loop() -> void
{
    hw::run(workout, config, state);
}
