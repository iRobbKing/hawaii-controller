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
        .controller_mac = { 0x02, 0x12, 0x34, 0x56, 0x78, 0x9A },
        .controller_address = IPAddress{192, 168, 31, 111},
        .controller_port = 5000,
        .controller_id = 1,
        .server_address = IPAddress{192, 168, 31, 166},
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

    hw::init(workout, config);
}

auto loop() -> void
{
    hw::run(workout, config, state);
}
