#include <Arduino.h>

#include "workout.hpp"

// TODO: log? error  on overflow
// TODO: handle errors, dont stop loop, it will disable punchbag forever
// TODO: handle multiple interrupts
// TODO: publish queue

namespace hw = hawaii::workout;
namespace hwa = hw::accelerator;
namespace hwc = hw::connection;
namespace hwl = hw::lamp;

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
        { { CONTROLLER_MAC }, IPAddress{CONTROLLER_IP}, IPAddress{SERVER_IP}, 1883, "c" TOSTRING(CONTROLLER_ID) },
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
        hwl::set_color(workout.lamp, 0xFFFF0000);
        Serial.print("Error: ");
        Serial.print(static_cast<int>(error.cause));
        Serial.print(' ');
        Serial.println(error.payload.erased);
    }
}

auto loop() -> void
{
    unsigned long const now = millis();

    if (hw::run(workout, config, state, now))
    {
        if (is_error)
            hwl::set_color(workout.lamp, 0);
        is_error = false;
    }
    else 
    {
        if (!is_error)
            hwl::set_color(workout.lamp, 0xFFFF0000);
        is_error = true;
    }
}
