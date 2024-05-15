
#include <Arduino.h>

#include <string>

#ifndef CLOCK_H_
#define CLOCK_H_

class Clock {
   public:
    void setTime(int hours, int minutes);
    void setDate(int year, int month, int day, int dayOfWeek);
    std::string timeToText();
    std::string yearToText();
    std::string dayOfWeekToText();
    std::string dateToText();

   private:
    int _minutes;
    int _hours;
    int _year;
    int _month;
    int _day;
    int _dayOfWeek;
};

#endif