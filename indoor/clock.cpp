#include "clock.h"

#include <string>

std::string timeToText(int hours, int minutes) {
    char time[5];
    sprintf(time, "%02d:%02d", hours, minutes);
    return std::string(time);
}

void Clock::addOneMinute() {
    _minutes++;
    if (_minutes >= 60) {
        _minutes = 0;
        _hours++;
        if (_hours >= 24) {
            _hours = 0;
        }
    }
}

void Clock::setTime(int hours, int minutes) {
    _hours = hours;
    _minutes = minutes;
}
void Clock::setSunrise(int hours, int minutes) {
    _hoursSunrise = hours;
    _minutesSunrise = minutes;
}

void Clock::setSunset(int hours, int minutes) {
    _hoursSunset = hours;
    _minutesSunset = minutes;
}

void Clock::setDate(int year, int month, int day, int dayOfWeek) {
    _year = year;
    _month = month;
    _day = day;
    _dayOfWeek = dayOfWeek;
}

std::string Clock::clockToText() { return timeToText(_hours, _minutes); }

std::string Clock::sunsetToText() { return timeToText(_hoursSunset, _minutesSunset); }

std::string Clock::sunriseToText() { return timeToText(_hoursSunrise, _minutesSunrise); }

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

void Clock::setWeather(int weather) { _weather = weather; }
void Clock::setWeatherJ1(int weather) { _weatherJ1 = weather; }
void Clock::setWeatherJ2(int weather) { _weatherJ2 = weather; }

const int STORM = 0, RAIN = 1, SNOW = 2, FOG = 3, SUN = 4, SUNNY_CLOUD = 5, CLOUD = 6, DRIZZLE = 7;

std::string wToText(int weather) {
    switch (weather) {
        {
            case 0:
                return "P";
            case 1:
                return "R";
            case 2:
                return "W";
            case 3:
                return "M";
            case 4:
                return "B";
            case 5:
                return "H";
            case 6:
                return "Y";
            case 7:
                return "Q";

            default:
                break;
        }
    }
}

std::string Clock::weatherToText() { return wToText(_weather); }
std::string Clock::weatherJ1ToText() { return wToText(_weatherJ1); }
std::string Clock::weatherJ2ToText() { return wToText(_weatherJ2); }