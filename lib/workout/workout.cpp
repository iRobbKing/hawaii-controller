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

    auto get_acceleration(hw::System &workout, hw::Config &config, hw::State &state, unsigned long const now, float &acceleration) -> bool
    {
        // if (!ha::get_acceleration(workout.accelerator, config.accelerator, acceleration))
        // {
        //     restart_wire();
        //     ha::reset(workout.accelerator);
        //     state.restarted = true;
        //     return false;
        // }

        if (hw::HIT_DEBOUNCE_TIME_MS <= now - state.last_hit_time_ms)
        {
            acceleration = 0.5f;
            state.last_hit_time_ms = now;

            return true;
        }

        return false;

        // if (state.restarted)
        // {
        //     state.restarted = false;
        //     hc::send_restarted(workout.connection, config.connection);
        //     return false;
        // }
        
        // if (acceleration < hw::NOISE_LIMIT)
        // {
        //     state.punchbag_acceleration = 0;
        //     state.delta = hw::AccelerationDelta::Noise;
        //     return false;
        // }

        // if (acceleration <= state.punchbag_acceleration)
        // {
        //     state.punchbag_acceleration = acceleration;
        //     state.delta = hw::AccelerationDelta::Decreasing;
        //     return false;
        // }

        // if (state.delta != hw::AccelerationDelta::Increasing)
        // {
        //     state.punchbag_acceleration = acceleration;
        //     state.delta = hw::AccelerationDelta::Increasing;

        //     if (hw::HIT_DEBOUNCE_TIME_MS <= now - state.last_hit_time_ms)
        //     {
        //         state.last_hit_time_ms = now;

        //         return true;
        //     }
        // }

        // return false;
    }
}

namespace hawaii::workout
{
    auto init(System &workout, Config &config) -> void
    {
        lamp::init(workout.lamp, config.lamp);

        accelerator::init(workout.accelerator, config.accelerator);

        connection::init(workout.connection, config.connection);
    }

    auto run(System &workout, Config &config, State &state) -> void
    {
        unsigned long const now = millis();

        if (state.need_to_clear_color && state.clear_color_in <= now - state.set_color_at)
        {
            state.need_to_clear_color = false;
            lamp::set_color(workout.lamp, 0);
        }

        // connection::Command command;
        // if (connection::get_message(workout.connection, command))
        // {
        //     switch (command.type)
        //     {
        //         case connection::CommandType::SetColor:
        //         {
        //             if (command.payload.set_color.controller_id != 0 && command.payload.set_color.controller_id != config.connection.controller_id)
        //                 break;

        //             lamp::set_color(workout.lamp, command.payload.set_color.color);
        //             state.need_to_clear_color = true;
        //             state.set_color_at = now;
        //             state.clear_color_in = command.payload.set_color.duration_ms;
        //             break;
        //         }
        //         case connection::CommandType::Reboot:
        //         {
        //             if (command.payload.reboot.controller_id != 0 && command.payload.reboot.controller_id != config.connection.controller_id)
        //                 break;

        //             ha::reset(workout.accelerator);
        //             wdt_enable(WDTO_15MS);
        //             while (true) {}
        //             break;
        //         }
        //         case connection::CommandType::ToggleDevMode:
        //         {
        //             state.show_hit = !state.show_hit;
        //             break;
        //         }
        //         case connection::CommandType::StartFitboxingRound:
        //         {
        //             state.fitboxing_started_at = now;
        //             break;
        //         }
        //     }
        // }
        
        // if (state.need_to_show_me)
        // {
        //     state.need_to_show_me = false;
        //     lamp::set_color(workout.lamp, 0xFF00FFFF);
        //     state.need_to_clear_color = true;
        //     state.set_color_at = now;
        //     state.clear_color_in = 3000;
        // }

        // if (5000 < now - state.last_ping_time)
        // {
        //     state.last_ping_time = now;
        //     connection::send_ping(workout.connection, config.connection, state.sent_hit_packets);
        // }

        float acceleration;
        if (get_acceleration(workout, config, state, now, acceleration))
        {
            // if (state.show_hit)
            // {
            //     hl::set_color(workout.lamp, 0xFFFF00FF);
            //     state.need_to_clear_color = true;
            //     state.set_color_at = now;
            //     state.clear_color_in = 200;
            // }

            connection::send_acceleration(workout.connection, config.connection, acceleration);
            state.sent_hit_packets += 1;
        }
    }
}
