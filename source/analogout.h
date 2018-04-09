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
#ifndef ANALOGOUT_H
#define ANALOGOUT_H

#include <array>
#include "ch_extended.h"
#include "hal.h"
#include "type_traits_ex.h"
#include "chprintf.h"

namespace Analog {

  class OutputCommand
  {
  private:
    static constexpr size_t chNumber_ = 4;
    static constexpr size_t maxMask_ = Utils::NumberToMask_v<chNumber_>;
    std::array<uint16_t, 4> values_;
    uint16_t channelMask_;
  public:
    static constexpr size_t GetChannelNumber()
    {
      return chNumber_;
    }
    uint16_t GetChannelMask() const
    {
      return channelMask_;
    }
    uint16_t GetValue(size_t ch) const
    {
      return values_[ch];
    }
    Rtos::Status SetValue(size_t ch, uint16_t value)
    {
      if(ch > chNumber_) {
        return Rtos::Status::Failure;
      }
      channelMask_ |= uint16_t(1U << ch);
      values_[ch] = value;
      return Rtos::Status::Success;
    }
    void Clear()
    {
      channelMask_ = 0;
    }
  };

  class Output : Rtos::BaseStaticThread<256>
  {
  private:
    static const PWMConfig pwmcfg;
    static PWMDriver* const PWMD;
    void main() override
    {
      while(true) {
        /* Waiting for a queued message then retrieving it.*/
        thread_t *tp = chMsgWait();
        const OutputCommand& cmd = *reinterpret_cast<const OutputCommand*>(chMsgGet(tp));
        chMsgRelease(tp, MSG_OK);
        auto chMask = cmd.GetChannelMask();
        for(pwmchannel_t i{}; i < cmd.GetChannelNumber(); ++i) {
          if(chMask & (1U << i)) {
            chprintf((BaseSequentialStream*)&SD1, "i: %u  val: %u\r\n", i, cmd.GetValue(i));
            SetValue(i, cmd.GetValue(i));
          }
        }
      }
    }
    void SetValue(pwmchannel_t ch, pwmcnt_t value)
    {
      pwmEnableChannel(PWMD, ch, value);
    }
  public:
    void Init()
    {
      IOBus pwmBus{GPIOA, 0x0F, 8};
      palSetBusMode(&pwmBus, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
      pwmStart(PWMD, &pwmcfg);
      start(NORMALPRIO);
    }
    msg_t SendMessage(const OutputCommand& msg)
    {
      return chMsgSend(thread_ref, reinterpret_cast<msg_t>(&msg));
    }

  };
}

#endif // ANALOGOUT_H
