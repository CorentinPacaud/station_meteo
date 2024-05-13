
#include <Arduino.h>

#ifndef CLOCK_H_
#define CLOCK_H_

class Clock {
   public:
    void setTime(int hours, int minutes);
    void setDate(int year, int month, int day, int dayOfWeek);
    String timeToText();
    String yearToText();
    String dayOfWeekToText();
    String dateToText();

   private:
    int _minutes;
    int _hours;
    int _year;
    int _month;
    int _day;
    int _dayOfWeek;
};

#endif