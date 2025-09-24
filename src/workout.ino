#include <Arduino.h>

#include "workout.hpp"

// TODO: log? error  on overflow
// TODO: handle errors, dont stop loop, it will disable punchbag forever
// TODO: handle multiple interrupts
// TODO: publish queue

namespace hw = hawaii::workout;
namespace hwa = hw::accelerator;
namespace hwc = hw::connection;
namespace hwm = hw::monitor;

hw::Config config{};
hw::System workout{};
hw::State state{};

hw::Error error;

namespace
{
    auto log_error(hw::Error error) -> void
    {
        if (error.cause == hw::ErrorCause::Accelerator)
            hwm::print(workout.monitor, "Что-то с датчиком :D");
        else if (error.cause == hw::ErrorCause::Connection)
            hwm::print(workout.monitor, "Что-то с сетью :D");
        else
            hwm::print(workout.monitor, String((int)error.cause, 10) + " " + String((int)error.payload.erased, 10));
    }
}

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

auto setup() -> void
{
    error.cause = hw::ErrorCause::None;
    error.payload.erased = 0;

    config = hw::Config{
        { ACCEL_FS::A8G },
        { { MAC }, IPAddress{192, 168, 2, 128}, IPAddress{SERVER}, 1883, TOSTRING(CLIENT_ID), "testuser", "123" },
        { 24,  6, 255 }
    };

    Wire.begin();
    Wire.setWireTimeout(5000);

    hw::Error e = hw::init(workout, config);
    if (e.cause != hw::ErrorCause::None)
    {
        log_error(e);
        error = e;
        return;
    }
}

auto loop() -> void
{
    if (error.cause != hw::ErrorCause::None)
    {
        if (hw::handle_error(workout, config, error))
        {
            error.cause = hw::ErrorCause::None;
            error.payload.erased = 0;
        }
        else
        {
            return;
        }

    }

    hw::Error e = hw::run(workout, config, state);
    if (e.cause != hw::ErrorCause::None)
    {
        log_error(e);
        error = e;
        return;
    }

    workout.connection.mqtt.loop();
}
