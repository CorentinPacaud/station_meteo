
#include <Arduino.h>

#ifndef CLOCK_H_
#define CLOCK_H_

class Clock {
   public:
    void setTime(int hours, int minutes);
    String toText();

   private:
    int _minutes;
    int _hours;
};

#endif