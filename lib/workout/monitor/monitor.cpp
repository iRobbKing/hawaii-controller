#include "monitor.hpp"


namespace hawaii::workout::monitor
{
    auto init(System& monitor) -> void
    {
        monitor.oled = iarduino_OLED_txt(0x3C);

        monitor.oled.begin();
        monitor.oled.setFont(SmallFontRus);
        monitor.oled.clrScr();
    }

    auto print(System& monitor, String message) -> void
    {
        monitor.oled.clrScr();

        monitor.oled.print(message, OLED_C, 0);
    }
}
