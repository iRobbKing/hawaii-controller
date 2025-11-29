#ifndef HAWAII_CONTROLLER_LAMP_HPP
#define HAWAII_CONTROLLER_LAMP_HPP

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

namespace hawaii::lamp
{
    struct Config
    {
        byte light_quantity;
        byte light_pin;
        byte light_brightness;
    };

    struct System
    {
        Adafruit_NeoPixel light;
    };

    auto init(System &lamp, Config &config) -> void;
    auto set_color(System &lamp, uint32_t color) -> void;
}

#endif