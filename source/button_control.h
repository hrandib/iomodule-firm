#ifndef BUTTON_CONTROL_H
#define BUTTON_CONTROL_H

#include "hal.h"

namespace Wk {

template<typename LedDriver>
class ButtonControl {
private:
  enum {
    BLIND_TIMESLOT = 1,
    SHORTPRESS_TIMESLOT = 15,
    UPDATE_PERIOD_MS = 40
  };

  ioportid_t buttonPort_;
  uint16_t buttonPin_;
  uint8_t direction{};
  uint8_t count{};
public:
  ButtonControl(ioportid_t port, uint16_t pin) : buttonPort_{port}, buttonPin_{pin}
  {
    palSetPadMode(port, pin, PAL_MODE_INPUT_PULLDOWN);
  }
  void Update()
  {
    if(palReadPad(buttonPort_, buttonPin_)) {
      ++count;
      if(count > SHORTPRESS_TIMESLOT) {
        if(!direction) {
          LedDriver::Increment(1);
        }
        else {
          LedDriver::Decrement(1);
        }
      }
    }
    else if(count) {
      if(count >= BLIND_TIMESLOT && count <= SHORTPRESS_TIMESLOT) {
        LedDriver::ToggleOnOff();
      }
      else {
        direction ^= 1;
      }
      count = 0;
    }
  }
  constexpr static systime_t GetUpdatePeriod()
  {
    return MS2ST(UPDATE_PERIOD_MS);
  }
};

}//Wk

#endif // BUTTON_CONTROL_H
