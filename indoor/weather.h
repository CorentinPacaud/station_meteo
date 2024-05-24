#include <string>

#ifndef WEATHER_H_
#define WEATHER_H_

class Temperature {
   public:
    int _currentTemperature;
    int _minTemperature;
    int _maxTemperature;
    int _humidity;
};

class Weather {
   public:
    Weather();
    void setIndoorTemperature(int current, int max, int min);
    void setOutdoorTemperature(int current, int max, int min);

    Temperature _indoorTemperature;
    Temperature _outdoorTemperature;
};

#endif