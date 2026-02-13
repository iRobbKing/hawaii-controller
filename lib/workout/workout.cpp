#include "workout.hpp"
#include <avr/wdt.h>

namespace
{
    namespace hw = hawaii::workout;
    namespace hwa = hw::accelerator;
    namespace hwl = hw::lamp;

    auto get_acceleration(hw::System &workout, hw::Config &config, hw::State &state, unsigned long const now, float &acceleration) -> bool
    {
        if (!hwa::get_acceleration(workout.accelerator, config.accelerator, acceleration))
        {
            Wire.clearWireTimeoutFlag();

            Wire.end();
            delay(30);
            Wire.begin();

            Wire.setWireTimeout();

            hwa::reinit(workout.accelerator);

            return false;
        }

        if (acceleration < hw::NOISE_LIMIT)
        {
            state.punchbag_acceleration = 0;
            state.delta = hw::AccelerationDelta::Noise;
            return false;
        }

        if (acceleration <= state.punchbag_acceleration)
        {
            state.punchbag_acceleration = acceleration;
            state.delta = hw::AccelerationDelta::Decreasing;
            return false;
        }

        if (state.delta != hw::AccelerationDelta::Increasing)
        {
            state.punchbag_acceleration = acceleration;
            state.delta = hw::AccelerationDelta::Increasing;

            if (hw::HIT_DEBOUNCE_TIME_MS <= now - state.last_hit_time_ms)
            {
                state.last_hit_time_ms = now;

                return true;
            }
        }

        return false;
    }
}

namespace hawaii::workout
{
    auto init(System &workout, Config &config) -> Error
    {
        Error error;
        error.cause = ErrorCause::None;
        error.payload.erased = 0;

        lamp::init(workout.lamp, config.lamp);
        lamp::set_color(workout.lamp, 0xFFFFFF00);

        accelerator::init(workout.accelerator, config.accelerator);

        connection::Error const connection_error = connection::init(workout.connection, config.connection);

        wdt_enable(WDTO_8S);

        if (connection_error != connection::Error::None) {
            error.cause = ErrorCause::Connection;
            error.payload.connection = connection_error;
            return error;
        }

        lamp::set_color(workout.lamp, 0);

        return error;
    }

    auto run(System &workout, Config &config, State &state, unsigned long const now) -> bool
    {
        wdt_reset();

        if (!connection::loop(workout.connection, config.connection, now))
            return false;

        if (workout.need_to_clear_color && workout.clear_color_in <= now - workout.set_color_at)
        {
            workout.need_to_clear_color = false;
            lamp::set_color(workout.lamp, 0);
        }

        uint32_t setcolor_color;
        if (connection::try_get_setcolor(setcolor_color))
        {
            lamp::set_color(workout.lamp, setcolor_color);
            workout.need_to_clear_color = true;
            workout.set_color_at = now;
            workout.clear_color_in = 444;
        }
        
        connection::try_get_dev_mode(workout.show_hit);

        if (workout.need_to_show_me)
        {
            workout.need_to_show_me = false;
            lamp::set_color(workout.lamp, 0xFF00FFFF);
            workout.need_to_clear_color = true;
            workout.set_color_at = now;
            workout.clear_color_in = 3000;
        }

        if (5000 < now - state.last_ping_time)
        {
            state.last_ping_time = now;
            connection::send_ping(workout.connection, config.connection);
        }

        float acceleration;
        if (get_acceleration(workout, config, state, now, acceleration))
        {
            if (workout.show_hit)
            {
                hwl::set_color(workout.lamp, 0xFFFF00FF);
                workout.need_to_clear_color = true;
                workout.set_color_at = now;
                workout.clear_color_in = 200;
            }

            connection::send_acceleration(workout.connection, config.connection, acceleration);
        }

        return true;
    }
}
