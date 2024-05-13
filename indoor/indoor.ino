// ESP-32
#define EPD_CS SS
#include <SPI.h>

#include "background.h"
#include "clock.h"
#include "config.h"
#include "screen.h"

Clock myClock;
GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(EPD_CS, 17, 16, 4));
Screen* myScreen = new Screen(myClock, display);

void setup() {
    Serial.begin(115200);
    myScreen->init();

    myClock.setTime(10, 53);

    myScreen->refresh();
}

void loop() {
    /*  Serial.println(
         "TEST"); */
}
