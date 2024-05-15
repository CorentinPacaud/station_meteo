// ESP-32
#define EPD_CS SS
#include <SPI.h>

#include "clock.h"
#include "clock.service.h"
#include "screen.h"

Clock myClock;
GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(EPD_CS, 17, 16, 4));
Screen* myScreen = new Screen(&myClock, display);
ClockService* clockService = new ClockService(&myClock);

void setup() {
    Serial.begin(115200);
    clockService->getData();

    myScreen->init();

    // myClock.setTime(10, 53);
    // myClock.setDate(2024, 5, 13, 1);

    myScreen->refresh();
}

void loop() {
    /*  Serial.println(
         "TEST"); */
}
