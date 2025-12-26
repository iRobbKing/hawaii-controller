#ifndef HAWAII_WORKOUT_H
#define HAWAII_WORKOUT_H

#include <avr/wdt.h>
#include <Wire.h>
#include "../accelerator/accelerator.hpp"
#include "../connection/connection.hpp"
#include "../lamp/lamp.hpp"

namespace hawaii::workout
{
    float constexpr NOISE_LIMIT = 0.1f;

    enum struct FitboxingDelta
    {
        Noise,
        Increasing,
        Decreasing,
    };

    struct Fitboxing
    {
        uint8_t max_rounds = 6;
        uint8_t current_round = 0;
        uint64_t current_round_started_at_ms = 100000;
        uint8_t max_series = 30;
        uint8_t current_series = 0;
        uint8_t max_tact = 8;
        uint8_t current_tact = 0;
        uint16_t tact_duration_ms = 444;
        float max_punchbag_acceleration = 0;
        float last_max_punchbag_acceleration = 0;
        bool passed_threshold = true;
        float passed_threshold_with = 0;
    };

    // struct Fitboxing
    // {
    //     uint8_t max_rounds = 0;
    //     uint8_t current_round = 0;
    //     uint64_t current_round_started_at_ms = 0;
    //     uint8_t max_series = 0;
    //     uint8_t current_series = 0;
    //     uint8_t max_tact = 0;
    //     uint8_t current_tact = 0;
    //     uint16_t tact_duration_ms = 0;
    //     float max_punchbag_acceleration = 0;
    // };

    struct State
    {
        uint64_t set_color_at;
        uint64_t clear_color_in;
        unsigned long last_ping_time = 0;
        bool need_to_clear_color = false;
        unsigned long long sent_hit_packets = 0;
        Fitboxing fitboxing;
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

    auto init(System &workout, Config &config, State &state) -> void;
    auto run(System &workout, Config &config, State &state) -> void;
}

#endif
