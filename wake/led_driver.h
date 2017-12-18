/*
 * Copyright (c) 2017 Dmytro Shestakov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include "hal.h"
#include "wake_base.h"

extern Rtos::Mailbox<int32_t, 4> dispMsgQueue;

namespace Wk {

  static constexpr uint8_t MAX_BRIGHTNESS_VALUE = 100;
  static constexpr uint8_t DEFAULT_BRIGHTNESS = 50;
  static constexpr uint8_t BRIGHTNESS_CHANGE_STEP_TIME = 25;

struct LedDriverFeatures
{
  enum Features {
    NoFeatures,
    FanControl = 1U << 1,
    TwoChannels = 1U << 0
  };
  static const PWMConfig pwmcfg;
  static PWMDriver* const PWMD;
  static const uint16_t LUT[MAX_BRIGHTNESS_VALUE + 1];

  static const ioportid_t pwmPort;
  static const uint16_t pwmPad;

  static constexpr Features features{ NoFeatures };
};

constexpr LedDriverFeatures::Features operator|(LedDriverFeatures::Features f1, LedDriverFeatures::Features f2)
{
  return static_cast<LedDriverFeatures::Features>(uint32_t(f1) | uint32_t(f2));
}

template<typename Features = LedDriverFeatures>
class LedDriver : public Wk::NullModule
{
private:
  enum InstructionSet {
      C_GetState = 16,
      C_GetBright = 16,
      C_GetFan = 16,
      C_SetBright = 17,
      C_IncBright = 18,
      C_DecBright = 19,
      C_SetFan = 20,
      C_SetGfgFanAuto = 21
  };

  static virtual_timer_t changeVt_;
  static uint8_t currentValue_;
  static uint8_t previousValue_;

  static void ChangeCb(void* arg)
  {
    const uint32_t newValue = reinterpret_cast<uint32_t>(arg);
    if(currentValue_ < newValue) {
      ++currentValue_;
    }
    else if(currentValue_ > newValue) {
      --currentValue_;
    }
    else {
      return;
    }
    Rtos::SysLockGuardFromISR lock;
    pwmEnableChannelI(Features::PWMD, 2, Features::LUT[currentValue_]);
    chVTSetI(&changeVt_, MS2ST(BRIGHTNESS_CHANGE_STEP_TIME), ChangeCb, arg);
  }
public:
  static void SetBrightness(uint8_t val) {
    if(val > MAX_BRIGHTNESS_VALUE) {
      val = MAX_BRIGHTNESS_VALUE;
    }
    dispMsgQueue.post(val, TIME_IMMEDIATE);
    chVTSet(&changeVt_, MS2ST(BRIGHTNESS_CHANGE_STEP_TIME), ChangeCb, (void*)(uint32_t)val);
  }
  static uint8_t GetBrightness() {
    return currentValue_;
  }

  static uint8_t Increment(uint8_t step)
  {
    uint8_t val = GetBrightness();
    if(val < MAX_BRIGHTNESS_VALUE) {
      val += step;
      SetBrightness(val);
    }
    else {
      val = MAX_BRIGHTNESS_VALUE;
    }
    return val;
  }
  static uint8_t Decrement(uint8_t step)
  {
    uint8_t val = GetBrightness();
    if(val <= step) {
      val = 0;
    }
    else {
      val -= step;
    }
    SetBrightness(val);
    return val;
  }

  static void Init()
  {
    palSetPadMode(Features::pwmPort, Features::pwmPad, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
    pwmStart(Features::PWMD, &Features::pwmcfg);
    chVTObjectInit(&changeVt_);
  }
  static uint8_t GetDeviceMask()
  {
    return Wk::DevLedDriver;
  }
  static uint8_t GetDeviceFeatures()
  {
    return Features::features;
  }
  static void On()
  {
    SetBrightness(previousValue_);
  }
  static void Off()
  {
    if(!GetBrightness()) {
      return;
    }
    previousValue_ = GetBrightness();
    SetBrightness(0);
  }
  static void ToggleOnOff()
  {
    (GetBrightness() ? Off : On)();
  }
  static bool Process(Wk::WakeBase::Packet& packet)
  {
    switch(packet.cmd) {
    case C_GetState:
      if(!packet.payloadSize) {
        packet.buf[0] = Wk::ERR_NO;
        packet.buf[1] = GetBrightness();
        packet.payloadSize = 2;
      }
      else {
        packet.ParameterError();
      }
      break;
    case C_SetBright:
      if(packet.payloadSize == 1) {
        SetBrightness(packet.buf[0]);
        packet.buf[0] = Wk::ERR_NO;
      }
      else {
        packet.ParameterError();
      }
      break;
    case C_IncBright:
      if(packet.payloadSize == 1) {
        packet.buf[1] = Increment(packet.buf[0]);
        packet.buf[0] = Wk::ERR_NO;
        packet.payloadSize = 2;
      }
      else {
        packet.ParameterError();
      }
      break;
    case C_DecBright:
      if(packet.payloadSize == 1) {
        packet.buf[1] = Decrement(packet.buf[0]);
        packet.buf[0] = Wk::ERR_NO;
        packet.payloadSize = 2;
      }
      else {
        packet.ParameterError();
      }
      break;
    default:
      //command not processed
      return false;
    }
    //command processed
    return true;
  }
};

template<typename Features>
uint8_t LedDriver<Features>::currentValue_;
template<typename Features>
uint8_t LedDriver<Features>::previousValue_ = DEFAULT_BRIGHTNESS;
template<typename Features>
virtual_timer_t LedDriver<Features>::changeVt_;

} //Wk


#endif // LED_DRIVER_H
