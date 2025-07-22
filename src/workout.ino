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
    Serial.begin(9600);

    config = hw::Config{
        { ACCEL_FS::A8G },
        { { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xAD }, IPAddress{192, 168, 50, 1}, 1883, "arduinoClient332", "testuser", "123" },
    };

    Wire.begin();
    Serial.begin(9600);
    Serial.println("Scanning...");
    while (!Serial);

    for (byte address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0)
        {
            Serial.print("I2C device found at 0x");
            Serial.println(address, HEX);
        }
    }
    Serial.println("Scan complete.");

    Serial.println("---------");
    hw::Error error = hw::init(workout, config);
    Serial.println("=========");
    if (error.cause != hw::ErrorCause::None) {
        log_error(error);
        has_errored = true;
        return;
    }
}

auto loop() -> void
{
    if (has_errored) return;

    hw::Error error = hw::run(workout, config, state);
    if (error.cause != hw::ErrorCause::None) {
        log_error(error);
        has_errored = true;
        return;
    }

    workout.connection.mqtt.loop();
}
