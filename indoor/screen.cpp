#include "screen.h"

#include <Adafruit_GFX.h>

#include <string>

#include "Sono_Proportional_Regular12pt7b.h"
#include "Sono_Proportional_Regular18pt7b.h"
#include "Sono_Proportional_Regular24pt7b.h"
#include "Sono_Proportional_Regular30pt7b.h"
#include "Sono_Proportional_Regular50pt7b.h"
#include "background.h"
#include "meteocons20pt7b.h"
#include "meteocons25pt7b.h"
#include "meteocons40pt7b.h"

void Screen::init() { this->_display.init(115200, true, 2, false); }

void Screen::refresh() {
    this->_display.firstPage();
    do {
        this->_display.fillScreen(GxEPD_WHITE);

        this->_display.setTextColor(GxEPD_BLACK);

        this->showClock();
        this->showDate();
        this->showLines();
        this->showWeather();
        this->showTemperatures();
        this->showSunSetRise();

    } while (this->_display.nextPage());
}

void Screen::refreshFull() {
    this->_display.setFullWindow();

    refresh();
}

void Screen::refreshClock() {
    this->_display.setPartialWindow(0, 0, 250, 72);

    refresh();
}

void Screen::showClock() {
    this->_display.setTextColor(GxEPD_BLACK);
    this->_display.setFont(&Sono_Proportional_Regular50pt7b);
    this->_display.setCursor(5, 68);
    this->_display.print(this->_clock->clockToText().c_str());
}

void Screen::showDate() {
    const int top = 120;
    this->_display.setFont(&Sono_Proportional_Regular24pt7b);
    this->_display.setCursor(10, top);
    this->_display.print(this->_clock->yearToText().c_str());
    this->_display.setCursor(160, top);
    this->_display.print(this->_clock->dayOfWeekToText().c_str());
    this->_display.setCursor(270, top);
    this->_display.print(String(this->_clock->dateToText().c_str()));
}

void Screen::showLines() {
    this->_display.drawLine(0, 135, 400, 135, GxEPD_BLACK);
    this->_display.drawLine(146, 135, 146, 300, GxEPD_BLACK);
}

void Screen::showWeather() {
    this->_display.setFont(&meteocons40pt7b);
    this->_display.setCursor(30, 200);
    this->_display.print(this->_clock->weatherToText().c_str());
    this->_display.setFont(&meteocons20pt7b);
    this->_display.setCursor(40, 250);
    this->_display.print(this->_clock->weatherJ1ToText().c_str());
    this->_display.setCursor(40, 295);
    this->_display.print(this->_clock->weatherJ2ToText().c_str());

    this->_display.setFont(&Sono_Proportional_Regular12pt7b);
    this->_display.setCursor(90, 230);
    this->_display.print("J+1");

    this->_display.setCursor(90, 275);
    this->_display.print("J+2");
}

void Screen::showTemperatures() {
    this->_display.setFont(&Sono_Proportional_Regular30pt7b);

    // INDOOR =================================================================

    // icon
    this->_display.drawInvertedBitmap(150, 145, IMAGE_HOME, 24, 24, GxEPD_BLACK);

    this->_display.setFont(&Sono_Proportional_Regular30pt7b);
    this->_display.setCursor(200, 205);
    this->_display.print(this->_weather->_indoorTemperature._currentTemperature);

    // max
    this->_display.setFont(&Sono_Proportional_Regular18pt7b);
    this->_display.setCursor(225, 243);
    this->_display.print(this->_weather->_indoorTemperature._maxTemperature);

    // min
    this->_display.setFont(&Sono_Proportional_Regular18pt7b);
    this->_display.setCursor(225, 283);
    this->_display.print(this->_weather->_indoorTemperature._minTemperature);

    // humidity
    this->_display.setFont(&Sono_Proportional_Regular12pt7b);
    this->_display.setCursor(160, 263);
    char humStrIn[3];
    sprintf(humStrIn, "%02d%%", this->_weather->_indoorTemperature._humidity);
    this->_display.print(humStrIn);

    // OUTDOOR =================================================================

    // icon
    this->_display.drawInvertedBitmap(370, 145, IMAGE_NATURE, 24, 24, GxEPD_BLACK);

    this->_display.setFont(&Sono_Proportional_Regular30pt7b);
    this->_display.setCursor(300, 205);
    this->_display.print(this->_weather->_outdoorTemperature._currentTemperature);

    // max
    this->_display.setFont(&Sono_Proportional_Regular18pt7b);
    this->_display.setCursor(300, 243);
    this->_display.print(this->_weather->_outdoorTemperature._maxTemperature);

    // min
    this->_display.setFont(&Sono_Proportional_Regular18pt7b);
    this->_display.setCursor(300, 283);
    this->_display.print(this->_weather->_outdoorTemperature._minTemperature);

    // ICON TEMP
    this->_display.setFont(&meteocons25pt7b);
    this->_display.setCursor(255, 205);
    this->_display.print("'");

    // humidity
    this->_display.setFont(&Sono_Proportional_Regular12pt7b);
    this->_display.setCursor(355, 263);
    char humStrOut[3];
    sprintf(humStrOut, "%02d%%", this->_weather->_outdoorTemperature._humidity);
    this->_display.print(humStrOut);
}

void Screen::showSunSetRise() {
    this->_display.setFont(&meteocons25pt7b);
    this->_display.setCursor(253, 43);
    this->_display.print("1");
    this->_display.setFont(&Sono_Proportional_Regular18pt7b);
    this->_display.setCursor(307, 30);
    this->_display.print(this->_clock->sunriseToText().c_str());

    this->_display.setFont(&meteocons25pt7b);
    this->_display.setCursor(253, 80);
    this->_display.print("2");
    this->_display.setFont(&Sono_Proportional_Regular18pt7b);
    this->_display.setCursor(307, 68);
    this->_display.print(this->_clock->sunsetToText().c_str());
}