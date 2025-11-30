#include "workout.hpp"

namespace
{
    namespace hw = hawaii::workout;
    namespace ha = hawaii::accelerator;
    namespace hl = hawaii::lamp;

    auto get_acceleration(hw::System &workout, hw::Config &config, hw::State &state, unsigned long const now, float &acceleration) -> bool
    {
        if (!ha::get_acceleration(workout.accelerator, config.accelerator, acceleration))
        {
            Wire.clearWireTimeoutFlag();

            Wire.end();
            delay(30);
            Wire.begin();

            Wire.setWireTimeout();

            ha::reinit(workout.accelerator);

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

        accelerator::init(workout.accelerator, config.accelerator);

        connection::Error const connection_error = connection::init(workout.connection, config.connection);
        if (connection_error != connection::Error::None) {
            error.cause = ErrorCause::Connection;
            error.payload.connection = connection_error;
            return error;
        }

        return error;
    }

    auto run(System &workout, Config &config, State &state, unsigned long const now) -> void
    {
        connection::loop(workout.connection);

        if (workout.need_to_clear_color && workout.clear_color_in <= now - workout.set_color_at)
        {
            workout.need_to_clear_color = false;
            lamp::set_color(workout.lamp, 0);
        }

        connection::Command command;
        if (connection::get_message(workout.connection, command))
        {
            switch (command.type)
            {
                case connection::CommandType::SetColor:
                {
                    if (command.payload.set_color.controller_id != config.connection.controller_id)
                        break;

                    lamp::set_color(workout.lamp, command.payload.set_color.color);
                    workout.need_to_clear_color = true;
                    workout.set_color_at = now;
                    workout.clear_color_in = command.payload.set_color.duration_ms;
                    break;
                }
                case connection::CommandType::Reboot:
                {
                    if (command.payload.reboot.controller_id != config.connection.controller_id)
                        break;

                    wdt_enable(WDTO_15MS);
                    while (true) {}
                    break;
                }
                case connection::CommandType::ToggleDevMode:
                {
                    workout.show_hit = !workout.show_hit;
                    break;
                }
            }
        }
        
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
                hl::set_color(workout.lamp, 0xFFFF00FF);
                workout.need_to_clear_color = true;
                workout.set_color_at = now;
                workout.clear_color_in = 200;
            }

            connection::send_acceleration(workout.connection, config.connection, acceleration);
        }
    }
}
