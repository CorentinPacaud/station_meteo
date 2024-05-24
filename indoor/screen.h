#include "GxEPD2_GFX.h"
#include "clock.h"
#include "weather.h"

#ifndef DISPLAY_H_
#define DISPLAY_H_

/*
 *
 *
 * ESP8266 ================================================================
 *
 * SCREEN
 * BUSY (violet) => D2
 * RST (blanc)   => D4
 * DC (vert)     => D3
 * CS (orange)   => D8
 * CLK (jaune)   => D5
 * DIN (bleu)    => D7
 * GND (noir)    => gnd
 * VCC (rouge)   => 3.3v
 *
 * DHT22
 * DATA => D1
 *
 *
 * ESP32 =================================================================
 *
 * BUSY (violet)  => D4
 * RST (blanc)    => RX2
 * DC (vert)      => TX2
 * CS (orange)    => D5 (Pulldown 47kΩ => Neg )
 * CLK (jaune)    => D18
 * DIN (bleu)     => D23
 * GND (noir)     => GND
 * VCC (rouge)    => 3.3v
 *
 *
 *
 */

#include <GxEPD2_BW.h>

class Screen {
   private:
    GxEPD2_BW<GxEPD2_420, 300>& _display;
    Clock* _clock;
    Weather* _weather;

    void showClock();
    void showDate();
    void showLines();
    void showWeather();
    void showTemperatures();
    void showSunSetRise();
    void refresh();

   public:
    Screen(Clock* clock, Weather* weather, GxEPD2_BW<GxEPD2_420, 300>& display) : _clock(clock), _weather(weather), _display(display){};
    void init();
    void refreshFull();
    void refreshClock();
};
#endif