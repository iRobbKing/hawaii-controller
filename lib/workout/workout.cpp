#include "workout.hpp"

namespace hawaii::workout
{
    auto init(System &workout, Config &config) -> Error
    {
        lamp::init(workout.lamp, config.lamp);

        lamp::set_color(workout.lamp, 0xFFFFFF00);

        Error error;
        error.cause = ErrorCause::None;
        error.payload.erased = 0;

        Serial.println(1);
        accelerator::Error const accelerator_error = accelerator::init(workout.accelerator, config.accelerator);
        Serial.println(2);
        if (accelerator_error != accelerator::Error::None)
        {
            error.cause = ErrorCause::Accelerator;
            error.payload.accelerator = accelerator_error;
            return error;
        }

        Serial.println(3);
        connection::Error const connection_error = connection::init(workout.connection, config.connection);
        Serial.println(4);
        if (connection_error != connection::Error::None) {
            error.cause = ErrorCause::Connection;
            error.payload.connection = connection_error;
            return error;
        }

        lamp::set_color(workout.lamp, 0);

        return error;
    }

    auto handle_error(System &workout, Config const& config, Error error) -> bool
    {
        if (round(millis() / 1000) % 3 == 0)
            lamp::set_color(workout.lamp, 0xFFFF0000);
        else
            lamp::set_color(workout.lamp, 0);

        if (round(millis() / 1000) % 5 == 0 && error.cause == ErrorCause::Connection)
        {
            return connection::reconnect(workout.connection, config.connection);
        }

        return false;
    }

    auto run(System &workout, Config &config, State &state) -> Error
    {
        Error error;
        error.cause = ErrorCause::None;
        error.payload.erased = 0;

        uint64_t const now = millis();
        uint32_t setcolor_color;
        if (connection::try_get_setcolor(setcolor_color))
        {
            lamp::set_color(workout.lamp, setcolor_color);
            workout.need_to_clear_color = true;
            workout.set_color_at = now;
            workout.clear_color_in = 444;
        }
        
        connection::try_get_dev_mode(workout.show_hit);

        // TODO: fix overflow
        if (workout.need_to_clear_color && workout.clear_color_in <= now - workout.set_color_at)
        {
            workout.need_to_clear_color = false;
            lamp::set_color(workout.lamp, 0);
        }

        connection::try_get_show_me(workout.need_to_show_me);

        if (workout.need_to_show_me)
        {
            workout.need_to_show_me = false;
            lamp::set_color(workout.lamp, 0xFF00FFFF);
            workout.need_to_clear_color = true;
            workout.set_color_at = now;
            workout.clear_color_in = 3000;
        }

        if (round(millis() / 1000) % 5 == 0)
        {
            connection::Error const send_message_error = connection::send_ping(workout.connection, config.connection);
            if (send_message_error != connection::Error::None)
            {
                error.cause = ErrorCause::Connection;
                error.payload.connection = send_message_error;
                return error;
            }
        }

        // TODO: ticks from ddd? since we have to call this from int callback from loop
        // and only call get_acceleration if there is data

        float const acceleration = accelerator::get_acceleration(workout.accelerator, config.accelerator);

        if (acceleration < NOISE_LIMIT)
        {
            state.punchbag_acceleration = 0;
            state.delta = AccelerationDelta::Noise;
            return error;
        }

        if (acceleration <= state.punchbag_acceleration)
        {
            state.punchbag_acceleration = acceleration;
            state.delta = AccelerationDelta::Decreasing;
            return error;
        }

        if (state.delta != AccelerationDelta::Increasing)
        {
            state.punchbag_acceleration = acceleration;
            state.delta = AccelerationDelta::Increasing;

            unsigned long const now_ms = millis();
            if (HIT_DEBOUNCE_TIME_MS <= now_ms - state.last_hit_time_ms)
            {
                state.last_hit_time_ms = now_ms;

                if (workout.show_hit)
                {
                    lamp::set_color(workout.lamp, 0xFFFF00FF);
                    Serial.print(" Удар ");
                    Serial.print(acceleration);
                    Serial.println();
                    workout.need_to_clear_color = true;
                    workout.set_color_at = now_ms;
                    workout.clear_color_in = 200;
                }

                connection::Error const send_message_error = connection::send_acceleration(workout.connection, config.connection, acceleration);
                if (send_message_error != connection::Error::None) {
                    error.cause = ErrorCause::Connection;
                    error.payload.connection = send_message_error;
                    return error;
                }
            }

            return error;
        }

        return error;
    }
}
