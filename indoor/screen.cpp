#include "screen.h"

#include <Adafruit_GFX.h>

#include "Sono_Proportional_Regular18pt7b.h"
#include "Sono_Proportional_Regular30pt7b.h"
#include "Sono_Proportional_Regular50pt7b.h"
#include "meteocons25pt7b.h"
#include "meteocons40pt7b.h"

void Screen::init() { this->_display.init(115200, true, 2, false); }

void Screen::refresh() {
    this->_display.setFullWindow();

    this->_display.firstPage();
    do {
        this->_display.fillScreen(GxEPD_WHITE);
        this->_display.setTextColor(GxEPD_BLACK);

        this->_display.setFont(&Sono_Proportional_Regular50pt7b);
        this->_display.setCursor(2, 65);
        this->_display.print(this->_clock.toText());

        this->_display.setFont(&meteocons25pt7b);
        this->_display.setCursor(250, 43);
        this->_display.print("B");
        this->_display.setFont(&Sono_Proportional_Regular18pt7b);
        this->_display.setCursor(303, 30);
        this->_display.print("07:54");

        this->_display.setFont(&meteocons25pt7b);
        this->_display.setCursor(250, 80);
        this->_display.print("C");
        this->_display.setFont(&Sono_Proportional_Regular18pt7b);
        this->_display.setCursor(303, 68);
        this->_display.print("21:36");

        this->_display.setFont(&Sono_Proportional_Regular18pt7b);
        this->_display.setCursor(5, 100);
        this->_display.print("Lundi");
        this->_display.setCursor(5, 125);
        this->_display.print("03/05");

        this->_display.setFont(&meteocons40pt7b);
        this->_display.setCursor(3, 205);
        this->_display.print("L");
        this->_display.setFont(&meteocons25pt7b);
        this->_display.setCursor(15, 250);
        this->_display.print("L");
        this->_display.setCursor(15, 295);
        this->_display.print("L");

        this->_display.setFont(&Sono_Proportional_Regular30pt7b);
        this->_display.setCursor(100, 120);
        this->_display.print("14");
        this->_display.setFont(&meteocons25pt7b);
        this->_display.setCursor(150, 120);
        this->_display.print('*');

    } while (this->_display.nextPage());
}