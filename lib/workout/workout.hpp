#ifndef HAWAII_WORKOUT_H
#define HAWAII_WORKOUT_H

#include "accelerator/accelerator.hpp"
#include "connection/connection.hpp"
#include "lamp/lamp.hpp"
#include <Wire.h>

namespace hawaii::workout
{
    enum struct AccelerationDelta
    {
        Noise = 0,
        Increasing = 1,
        Decreasing = 2,
    };

    unsigned long constexpr HIT_DEBOUNCE_TIME_MS = 250;
    float constexpr NOISE_LIMIT = 0.3f;

    struct State
    {
        float punchbag_acceleration = 0;
        unsigned long last_hit_time_ms = 0;
        AccelerationDelta delta = AccelerationDelta::Noise;
        unsigned long last_ping_time = 0;
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
        uint64_t clear_color_in;
        bool show_hit = false;
        bool need_to_show_me = false;
    };

    [[nodiscard]] auto init(System &workout, Config &config) -> Error;
    [[nodiscard]] auto run(System &workout, Config &config, State &state, unsigned long now) -> bool;
}

#endif
