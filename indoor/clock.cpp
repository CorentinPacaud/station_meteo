#include "clock.h"

void Clock::setTime(int hours, int minutes) {
    _hours = hours;
    _minutes = minutes;
}

void Clock::setDate(int year, int month, int day, int dayOfWeek) {
    _year = year;
    _month = month;
    _day = day;
    _dayOfWeek = dayOfWeek;
}

String Clock::timeToText() { return String(_hours) + ":" + String(_minutes); }

String Clock::yearToText() { return String(_year); };

String Clock::dayOfWeekToText() {
    const String days[7] = {
        "DIM", "LUN", "MAR", "MER", "JEU", "VEN", "SAM",
    };

    return days[_dayOfWeek];
};

String Clock::dateToText() { return String(_day) + "/" + String(_month); }
