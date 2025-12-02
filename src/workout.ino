#include <Arduino.h>

#include "workout.hpp"

// TODO: log? error  on overflow
// TODO: handle errors, dont stop loop, it will disable punchbag forever
// TODO: handle multiple interrupts
// TODO: publish queue

namespace hw = hawaii::workout;
namespace ha = hawaii::accelerator;
namespace hc = hawaii::connection;
namespace hl = hawaii::lamp;

hw::Config config{};
hw::System workout{};
hw::State state{};

bool is_error = false;

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

auto setup() -> void
{
    config = hw::Config{
        { ACCEL_FS::A8G },
        { { 0x02, 0x12, 0x34, 0x56, 0x78, 0x9A }, IPAddress{192, 168, 31, 166}, 5111, 5000, 1 },
        { 24,  6, 255 }
    };

    Serial.begin(9600);
    while (!Serial);

    Wire.begin();
    Wire.setWireTimeout();

    hw::Error error = hw::init(workout, config);
    if (error.cause != hw::ErrorCause::None)
    {
        is_error = true;
        hl::set_color(workout.lamp, 0xFFFF0000);
        while (true) { }
    }
}

auto loop() -> void
{
    unsigned long const now = millis();

    hw::run(workout, config, state, now);
}
