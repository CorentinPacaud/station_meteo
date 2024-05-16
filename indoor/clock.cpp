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

std::string Clock::timeToText() {
    char time[5];
    sprintf(time, "%02d:%02d", _hours, _minutes);
    return std::string(time);
}

std::string Clock::yearToText() { return std::to_string(_year); };

std::string Clock::dayOfWeekToText() {
    const std::string DAYS[7] = {
        "DIM", "LUN", "MAR", "MER", "JEU", "VEN", "SAM",
    };

    return DAYS[_dayOfWeek];
};

std::string Clock::dateToText() {
    char date[5];
    sprintf(date, "%02d/%02d", _day, _month);

    return std::string(date);
}
