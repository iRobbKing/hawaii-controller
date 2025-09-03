#ifndef HAWAII_WORKOUT_H
#define HAWAII_WORKOUT_H

#include "accelerator/accelerator.hpp"
#include "connection/connection.hpp"
#include "lamp/lamp.hpp"

namespace hawaii::workout
{
    enum struct AccelerationDelta
    {
        Noise = 0,
        Increasing = 1,
        Decreasing = 2,
    };

    unsigned long constexpr HIT_DEBOUNCE_TIME_MS = 50;
    float constexpr NOISE_LIMIT = 0.5f;

    struct State
    {
        float punchbag_acceleration = 0;
        unsigned long last_hit_time_ms = HIT_DEBOUNCE_TIME_MS;
        AccelerationDelta delta = AccelerationDelta::Noise;
    };

    enum struct ErrorCause
    {
        None = 0,
        Accelerator = 1,
        Connection = 2,
    };

    union ErrorPayload
    {
        int erased;
        accelerator::Error accelerator;
        connection::Error connection;
    };

    struct Error
    {
        ErrorCause cause;
        ErrorPayload payload;
    };

    struct Config
    {
        accelerator::Config accelerator;
        connection::Config connection;
        lamp::Config lamp;
    };

    struct System
    {
        accelerator::System accelerator;
        connection::System connection;
        lamp::System lamp;
        bool need_to_clear_color = false;
        uint64_t set_color_at;
    };

    [[nodiscard]] auto init(System &workout, Config &config) -> Error;
    auto show_error(System &workout) -> void;
    [[nodiscard]] auto run(System &workout, Config &config, State &state) -> Error;
}

#endif
