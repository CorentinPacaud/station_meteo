#include "clock.h"

#include <string>

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

std::string Clock::timeToText() { return std::to_string(_hours) + ":" + std::to_string(_minutes); }

std::string Clock::yearToText() { return std::to_string(_year); };

std::string Clock::dayOfWeekToText() {
    const std::string DAYS[7] = {
        "DIM", "LUN", "MAR", "MER", "JEU", "VEN", "SAM",
    };

    return DAYS[_dayOfWeek];
};

std::string Clock::dateToText() {
    char monthStr[2];
    char dayStr[2];
    sprintf(monthStr, "%02d", _month);
    sprintf(dayStr, "%02d", _day);
    std::string date;
    date.append(dayStr);
    date.append("/");
    date.append(monthStr);
    return date;
}
