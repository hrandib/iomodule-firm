/*
 * Copyright (c) 2018 Dmytro Shestakov
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

#include "analogout.h"


namespace Analog {

  Output output;

  const PWMConfig Output::pwmcfg_ {
    8000000UL,                                    /* 4MHz PWM clock frequency.   */
    4096,                                         /* Initial PWM period 1ms.      */
    nullptr,
    {
      {PWM_OUTPUT_ACTIVE_HIGH, nullptr},
      {PWM_OUTPUT_ACTIVE_HIGH, nullptr},
      {PWM_OUTPUT_ACTIVE_HIGH, nullptr},
      {PWM_OUTPUT_ACTIVE_HIGH, nullptr}
    },
    0,
    0,
#if STM32_PWM_USE_ADVANCED
    0
#endif
  };

  void Output::main()
  {
    setName("AnalogOutput");
    while(true) {
      /* Waiting for a queued message then retrieving it.*/
      thread_t *tp = chMsgWait();
      OutputCommand& cmd = *reinterpret_cast<OutputCommand*>(chMsgGet(tp));
      auto chMask = cmd.GetChannelMask();
      for(pwmchannel_t i{}; i < cmd.GetChannelNumber(); ++i) {
        if(chMask & (1U << i)) {
          SetValue(i, cmd.GetValue(i));
        }
        else {
          cmd.SetValue(i, values_[i]);
        }
      }
      chMsgRelease(tp, MSG_OK);
    }
  }

  void Output::SetValue(pwmchannel_t ch, pwmcnt_t value)
  {
    pwmEnableChannel(PWMD_, ch, value);
    values_[ch] = static_cast<channel_array_t::value_type>(value);
  }

  void Output::Init()
  {
    palSetBusMode(const_cast<IOBus*>(&pwmBus_), PAL_MODE_STM32_ALTERNATE_PUSHPULL);
    pwmStart(PWMD_, &pwmcfg_);
    start(NORMALPRIO);
  }

  msg_t Output::SendMessage(OutputCommand& msg)
  {
    return chMsgSend(thread_ref, reinterpret_cast<msg_t>(&msg));
  }

  Output::~Output()
  {
    stop();
    pwmStop(PWMD_);
    palSetBusMode(const_cast<IOBus*>(&pwmBus_), PAL_MODE_INPUT_PULLDOWN);
  }

}
