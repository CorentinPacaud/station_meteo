
#include <Arduino.h>

#include <string>

#ifndef CLOCK_H_
#define CLOCK_H_

class Clock {
   public:
    void setTime(int hours, int minutes);
    void setDate(int year, int month, int day, int dayOfWeek);
    void setSunset(int hours, int minutes);
    void setSunrise(int hours, int minutes);
    void setWeather(int weather);
    void setWeatherJ1(int weather);
    void setWeatherJ2(int weather);
    void addOneMinute();
    std::string clockToText();
    std::string yearToText();
    std::string dayOfWeekToText();
    std::string dateToText();
    std::string sunriseToText();
    std::string sunsetToText();
    std::string weatherToText();
    std::string weatherJ1ToText();
    std::string weatherJ2ToText();

   private:
    int _minutes;
    int _hours;
    int _year;
    int _month;
    int _day;
    int _dayOfWeek;
    int _minutesSunrise;
    int _minutesSunset;
    int _hoursSunrise;
    int _hoursSunset;

    int _weather;
    int _weatherJ1;
    int _weatherJ2;
};

#endif