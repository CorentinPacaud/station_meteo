#include "screen.h"

#include <Adafruit_GFX.h>

#include "Sono_Proportional_Regular12pt7b.h"
#include "Sono_Proportional_Regular18pt7b.h"
#include "Sono_Proportional_Regular24pt7b.h"
#include "Sono_Proportional_Regular30pt7b.h"
#include "Sono_Proportional_Regular50pt7b.h"
#include "background.h"
#include "meteocons25pt7b.h"
#include "meteocons40pt7b.h"

void Screen::init() { this->_display.init(115200, true, 2, false); }

void Screen::refresh() {
    this->_display.setFullWindow();

    this->_display.firstPage();
    do {
        this->_display.fillScreen(GxEPD_WHITE);

        this->_display.setTextColor(GxEPD_BLACK);

        this->showTime();
        this->showDate();
        this->showLines();
        this->showWeather();
        this->showTemperatures();
        this->showSunSetRise();

    } while (this->_display.nextPage());
}

void Screen::showTime() {
    this->_display.setTextColor(GxEPD_BLACK);
    this->_display.setFont(&Sono_Proportional_Regular50pt7b);
    this->_display.setCursor(5, 68);
    this->_display.print(this->_clock->timeToText().c_str());
}

void Screen::showDate() {
    const int top = 120;
    this->_display.setFont(&Sono_Proportional_Regular24pt7b);
    this->_display.setCursor(10, top);
    this->_display.print(this->_clock->yearToText().c_str());
    this->_display.setCursor(160, top);
    this->_display.print(this->_clock->dayOfWeekToText().c_str());
    this->_display.setCursor(265, top);
    this->_display.print(String(this->_clock->dateToText().c_str()));
}

void Screen::showLines() {
    this->_display.drawLine(0, 135, 400, 135, GxEPD_BLACK);
    this->_display.drawLine(146, 135, 146, 300, GxEPD_BLACK);
}

void Screen::showWeather() {
    this->_display.setFont(&meteocons40pt7b);
    this->_display.setCursor(30, 205);
    this->_display.print("L");
    this->_display.setFont(&meteocons25pt7b);
    this->_display.setCursor(40, 250);
    this->_display.print("L");
    this->_display.setCursor(40, 295);
    this->_display.print("L");

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
    this->_display.print("21");

    // max
    this->_display.setFont(&Sono_Proportional_Regular18pt7b);
    this->_display.setCursor(220, 243);
    this->_display.print("22");

    // min
    this->_display.setFont(&Sono_Proportional_Regular18pt7b);
    this->_display.setCursor(220, 283);
    this->_display.print("19");

    // humidity
    this->_display.setFont(&Sono_Proportional_Regular12pt7b);
    this->_display.setCursor(165, 263);
    this->_display.print("19%");

    // OUTDOOR =================================================================

    // icon
    this->_display.drawInvertedBitmap(370, 145, IMAGE_NATURE, 24, 24, GxEPD_BLACK);

    this->_display.setFont(&Sono_Proportional_Regular30pt7b);
    this->_display.setCursor(300, 205);
    this->_display.print("14");

    // max
    this->_display.setFont(&Sono_Proportional_Regular18pt7b);
    this->_display.setCursor(300, 243);
    this->_display.print("18");

    // min
    this->_display.setFont(&Sono_Proportional_Regular18pt7b);
    this->_display.setCursor(300, 283);
    this->_display.print("10");

    // ICON TEMP
    this->_display.setFont(&meteocons25pt7b);
    this->_display.setCursor(255, 205);
    this->_display.print("'");

    // humidity
    this->_display.setFont(&Sono_Proportional_Regular12pt7b);
    this->_display.setCursor(355, 263);
    this->_display.print("20%");
}

void Screen::showSunSetRise() {
    this->_display.setFont(&meteocons25pt7b);
    this->_display.setCursor(253, 43);
    this->_display.print("1");
    this->_display.setFont(&Sono_Proportional_Regular18pt7b);
    this->_display.setCursor(303, 30);
    this->_display.print("07:54");

    this->_display.setFont(&meteocons25pt7b);
    this->_display.setCursor(253, 80);
    this->_display.print("2");
    this->_display.setFont(&Sono_Proportional_Regular18pt7b);
    this->_display.setCursor(303, 68);
    this->_display.print("21:36");
}