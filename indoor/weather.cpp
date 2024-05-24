#include "weather.h"

#include <string>

Weather::Weather() {
    _indoorTemperature = {0, 0, 0};
    _outdoorTemperature = {0, 0, 0};
}

void Weather::setIndoorTemperature(int current, int max, int min) {
    this->_indoorTemperature._currentTemperature = current;
    this->_indoorTemperature._maxTemperature = max;
    this->_indoorTemperature._minTemperature = min;
}

void Weather::setOutdoorTemperature(int current, int max, int min) {
    this->_outdoorTemperature._currentTemperature = current;
    this->_outdoorTemperature._maxTemperature = max;
    this->_outdoorTemperature._minTemperature = min;
}