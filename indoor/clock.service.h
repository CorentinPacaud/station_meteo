#include "clock.h"

class ClockService {
   private:
    Clock* _clock;

   public:
    ClockService(Clock* clock) : _clock(clock){};
    void getData();
};