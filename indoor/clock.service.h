#include "clock.h"

class ClockService {
   private:
    Clock* _clock;
    void getDate();
    void getWeather();

   public:
    ClockService(Clock* clock) : _clock(clock){};
    void getData();
};