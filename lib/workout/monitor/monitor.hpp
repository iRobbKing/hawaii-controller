#ifndef HAWAII_WORKOUT_MONITOR_H
#define HAWAII_WORKOUT_MONITOR_H

#include <Wire.h>
#include <iarduino_OLED_txt_I2C.h>
#include <iarduino_OLED_txt.h>

namespace hawaii::workout::monitor
{
    struct System
    {
        iarduino_OLED_txt oled;
    };

    auto init(System& monitor) -> void;
    auto print(System& monitor, String message) -> void;
}

#endif
