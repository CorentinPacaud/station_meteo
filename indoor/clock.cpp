#include "clock.h"

void Clock::setTime(int hours, int minutes) {
    _minutes = minutes;
    _hours = hours;
}

String Clock::toText() { return String(_hours) + ":" + String(_minutes); }
