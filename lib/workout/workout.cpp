#include "workout.hpp"

namespace hawaii::workout
{
    auto init(System &workout, Config &config) -> Error
    {
        Error error;
        error.cause = ErrorCause::None;
        error.payload.erased = 0;

        accelerator::Error const accelerator_error = accelerator::init(workout.accelerator, config.accelerator);
        if (accelerator_error != accelerator::Error::None)
        {
            error.cause = ErrorCause::Accelerator;
            error.payload.accelerator = accelerator_error;
            return error;
        }

        connection::Error const connection_error = connection::init(workout.connection, config.connection);
        if (connection_error != connection::Error::None) {
            error.cause = ErrorCause::Connection;
            error.payload.connection = connection_error;
            return error;
        }

        return error;
    }

    auto run(System &workout, Config &config, State &state) -> Error
    {
        Error error;
        error.cause = ErrorCause::None;
        error.payload.erased = 0;

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
            unsigned long const now_ms = millis();
            // if (HIT_DEBOUNCE_TIME_MS <= now_ms - state.last_hit_time_ms)
            {
                // state.last_hit_time_ms = now_ms;
                Serial.print(" Удар ");
                Serial.print(acceleration * 80.0f);
                Serial.println();
            }

            state.punchbag_acceleration = acceleration;
            state.delta = AccelerationDelta::Increasing;

            connection::Topic constexpr topic = "accelerator/acceleration";
            unsigned short const now = (unsigned short) millis();
            connection::Topic const payload = reinterpret_cast<connection::Topic>(&now);
            connection::Error const send_message_error = connection::send_message(workout.connection, topic, payload);
            if (send_message_error != connection::Error::None) {
                error.cause = ErrorCause::Connection;
                error.payload.connection = send_message_error;
                return error;
            }

            return error;
        }

        return error;
    }
}
