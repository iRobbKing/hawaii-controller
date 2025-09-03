#include "lamp.hpp"

namespace hawaii::workout::lamp
{
    auto init(System &lamp, Config &config) -> void
    {
        lamp.light = Adafruit_NeoPixel{config.light_quantity, config.light_pin, NEO_GRB + NEO_KHZ800};
        lamp.light.begin();
        lamp.light.clear();
        lamp.light.show();
        lamp.light.setBrightness(config.light_brightness);
    }

    auto set_color(System &lamp, uint32_t const color) -> void
    {
        for (byte i = 0; i < lamp.light.numPixels(); ++i)
            lamp.light.setPixelColor(i, color);

        lamp.light.show();
    }
}
