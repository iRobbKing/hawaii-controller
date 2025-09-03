#include <Arduino.h>

#include "workout.hpp"

// TODO: log? error  on overflow
// TODO: handle errors, dont stop loop, it will disable punchbag forever
// TODO: handle multiple interrupts
// TODO: publish queue

namespace hw = hawaii::workout;
namespace hwa = hw::accelerator;
namespace hwc = hw::connection;

hw::Config config{};
hw::System workout{};
hw::State state{};

bool has_errored = false;

namespace
{
    auto log_error(hw::Error error) -> void
    {
        Serial.print("Error: ");
        Serial.print(static_cast<int>(error.cause));
        Serial.print(" ");
        Serial.print(error.payload.erased);
        Serial.println();
    }
}

auto setup() -> void
{
    config = hw::Config{
        { ACCEL_FS::A8G },
        { { MAC }, IPAddress{192, 168, 2, 128}, IPAddress{SERVER}, 1883, "arduinoClient1", "testuser", "123" },
        { 24,  6, 255 }
    };

    Wire.begin();
    Serial.begin(9600);
    while (!Serial);

    hw::Error error = hw::init(workout, config);
    if (error.cause != hw::ErrorCause::None) {
        log_error(error);
        has_errored = true;
        return;
    }
}

auto loop() -> void
{
    if (has_errored) {
        hw::show_error(workout);
        return;
    }

    hw::Error error = hw::run(workout, config, state);
    if (error.cause != hw::ErrorCause::None) {
        log_error(error);
        has_errored = true;
        return;
    }

    workout.connection.mqtt.loop();
}
