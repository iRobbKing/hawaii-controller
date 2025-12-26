#include "workout.hpp"

namespace
{
    namespace hw = hawaii::workout;
    namespace ha = hawaii::accelerator;
    namespace hl = hawaii::lamp;
    namespace hc = hawaii::connection;

    auto restart_wire() -> void
    {
        Wire.clearWireTimeoutFlag();
        Wire.end();
        delay(30);
        Wire.begin();
        Wire.setWireTimeout(25000, true);
    }

    auto is_fitboxing_active(hw::Fitboxing const& fitboxing) -> bool
    {
        return fitboxing.current_round < fitboxing.max_rounds;
    }

    auto is_next_tact(hw::Fitboxing const& fitboxing, uint64_t const now) -> bool
    {
        uint64_t next_offset = (uint64_t)fitboxing.tact_duration_ms * ((uint64_t)fitboxing.current_series * (uint64_t)fitboxing.max_tact + (uint64_t)fitboxing.current_tact + 1);
        return fitboxing.current_round_started_at_ms + next_offset <= now;
    }

    auto advance_fitboxing(hw::Fitboxing& fitboxing) -> void
    {
        if (fitboxing.max_rounds <= fitboxing.current_round)
            return;

        fitboxing.current_tact += 1;
        if (fitboxing.max_tact <= fitboxing.current_tact)
        {
            fitboxing.current_tact = 0;
            fitboxing.current_series += 1;
            if (fitboxing.max_series <= fitboxing.current_series)
            {
                fitboxing.current_series = 0;
                fitboxing.current_round = fitboxing.max_rounds;
            }
        }

        fitboxing.last_max_punchbag_acceleration = fitboxing.max_punchbag_acceleration;
        fitboxing.passed_threshold = false;
        fitboxing.max_punchbag_acceleration = 0;
        fitboxing.passed_threshold_with = 0;
    }

    auto set_lamp_color(hw::System &workout, hw::State &state, uint32_t color, uint64_t now, uint64_t duration) -> void
    {
        hawaii::lamp::set_color(workout.lamp, color);
        state.need_to_clear_color = true;
        state.set_color_at = now;
        state.clear_color_in = duration;
    }
}

namespace hawaii::workout
{
    auto init(System &workout, Config &config, State& state) -> void
    {
        lamp::init(workout.lamp, config.lamp);

        accelerator::init(workout.accelerator, config.accelerator);

        connection::init(workout.connection, config.connection);

        state.fitboxing.current_round_started_at_ms = millis();
    }

    auto run(System &workout, Config &config, State &state) -> void
    {
        unsigned long const now = millis();

        if (state.need_to_clear_color && state.clear_color_in <= now - state.set_color_at)
        {
            state.need_to_clear_color = false;
            lamp::set_color(workout.lamp, 0);
        }

        connection::Command command;
        if (connection::get_message(workout.connection, command))
        {
            switch (command.type)
            {
                case connection::CommandType::SetColor:
                {
                    if (command.payload.set_color.controller_id != 0 && command.payload.set_color.controller_id != config.connection.controller_id)
                        break;

                    set_lamp_color(workout, state, command.payload.set_color.color, now, command.payload.set_color.duration_ms);
                    state.fitboxing.max_punchbag_acceleration = 0.9f;
                    break;
                }
                case connection::CommandType::StartFitboxingRound:
                {
                    state.fitboxing.max_rounds = command.payload.start_fitboxing_round.max_rounds;
                    state.fitboxing.current_round = command.payload.start_fitboxing_round.current_round;
                    state.fitboxing.current_round_started_at_ms = now;
                    state.fitboxing.max_series = command.payload.start_fitboxing_round.max_series;
                    state.fitboxing.current_series = 0;
                    state.fitboxing.max_tact = command.payload.start_fitboxing_round.max_tact;
                    state.fitboxing.current_tact = 0;
                    state.fitboxing.tact_duration_ms = command.payload.start_fitboxing_round.tact_duration_ms;
                    state.fitboxing.max_punchbag_acceleration = 0;
                    state.fitboxing.last_max_punchbag_acceleration = 0;
                    state.fitboxing.passed_threshold = true;
                    state.fitboxing.passed_threshold_with = 0;
                    break;
                }
            }
        }

        if (is_fitboxing_active(state.fitboxing) && is_next_tact(state.fitboxing, now))
        {
            if (NOISE_LIMIT <= state.fitboxing.max_punchbag_acceleration)
            {
                connection::send_acceleration(workout.connection, config.connection, state.fitboxing.current_round, state.fitboxing.current_series, state.fitboxing.current_tact, state.fitboxing.max_punchbag_acceleration);
                state.sent_hit_packets += 1;
            }

            advance_fitboxing(state.fitboxing);
        }

        if (5000 < now - state.last_ping_time)
        {
            state.last_ping_time = now;
            connection::send_ping(workout.connection, config.connection, state.sent_hit_packets);
        }

        float acceleration = 0;
        if (ha::get_acceleration(workout.accelerator, config.accelerator, acceleration))
        {
            if (!state.fitboxing.passed_threshold && (state.fitboxing.last_max_punchbag_acceleration * 1.25f <= acceleration || acceleration <= state.fitboxing.last_max_punchbag_acceleration * 0.85f))
            {
                state.fitboxing.passed_threshold = true;
                state.fitboxing.passed_threshold_with = acceleration;
            }

            if (state.fitboxing.passed_threshold && state.fitboxing.passed_threshold_with + NOISE_LIMIT <= acceleration && state.fitboxing.max_punchbag_acceleration < acceleration)
            {
                state.fitboxing.max_punchbag_acceleration = acceleration;
            }
        } 
        else
        {
            restart_wire();
            ha::reset(workout.accelerator);
        }
    }
}
