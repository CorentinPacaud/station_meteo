#include "clock.h"
#include "weather.h"

class ClockService {
   private:
    Clock* _clock;
    Weather* _weather;
    void getDate();
    void getWeather();

   public:
    ClockService(Clock* clock, Weather* weather) : _clock(clock), _weather(weather){};
    void getData();
};