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
    myScreen->init();

    while (true) {
        clockService->getData();

        myScreen->refreshFull();
        for (int min = 0; min < 5; min++) {
            for (int i = 0; i < 60; i++) {
                delay(1000);
            }
            // myClock.setTime(10, 53);
            // myClock.setDate(2024, 5, 13, 1);
            myClock.addOneMinute();
            myScreen->refreshClock();
        }
    }
}

void loop() {
    /*  Serial.println(
         "TEST"); */
}
