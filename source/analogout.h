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

  using channel_array_t = std::array<uint16_t, 4>;

  class OutputCommand
  {
  private:
    static constexpr size_t chNumber_ = 4;
    static constexpr size_t maxMask_ = Utils::NumberToMask_v<chNumber_>;
    channel_array_t values_;
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
    static const PWMConfig pwmcfg_;
    PWMDriver* const PWMD_;
    channel_array_t values_;
    const IOBus pwmBus_{GPIOA, 0x0F, 8};
    void main() override
    {
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
    void SetValue(pwmchannel_t ch, pwmcnt_t value)
    {
      pwmEnableChannel(PWMD_, ch, value);
      values_[ch] = static_cast<channel_array_t::value_type>(value);
    }
  public:
    Output() : PWMD_{&PWMD1}, values_{}
    { }
    void Init()
    {
      palSetBusMode(const_cast<IOBus*>(&pwmBus_), PAL_MODE_STM32_ALTERNATE_PUSHPULL);
      pwmStart(PWMD_, &pwmcfg_);
      start(NORMALPRIO);
    }
    msg_t SendMessage(OutputCommand& msg)
    {
      return chMsgSend(thread_ref, reinterpret_cast<msg_t>(&msg));
    }
    ~Output() override
    {
      stop();
      pwmStop(PWMD_);
      palSetBusMode(const_cast<IOBus*>(&pwmBus_), PAL_MODE_INPUT_PULLDOWN);
    }
  };

  extern Output output;
} //Analog

#endif // ANALOGOUT_H
