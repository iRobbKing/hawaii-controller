#ifndef HAWAII_WORKOUT_H
#define HAWAII_WORKOUT_H

#include <avr/wdt.h>
#include <Wire.h>
#include "../accelerator/accelerator.hpp"
#include "../connection/connection.hpp"
#include "../lamp/lamp.hpp"

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
        uint64_t set_color_at;
        uint64_t clear_color_in;
        float punchbag_acceleration = 0;
        unsigned long last_hit_time_ms = 0;
        AccelerationDelta delta = AccelerationDelta::Noise;
        unsigned long last_ping_time = 0;
        bool need_to_clear_color = false;
        bool show_hit = false;
        bool need_to_show_me = false;
        bool restarted = false;
        unsigned long long sent_hit_packets = 0;
        uint64_t fitboxing_started_at = 0;
        unsigned long last_message_check_time = 0;
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
    };

    auto init(System &workout, Config &config) -> void;
    auto run(System &workout, Config &config, State &state) -> void;
}

#endif
