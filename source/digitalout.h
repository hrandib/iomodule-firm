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
#ifndef DIGITALOUT_H
#define DIGITALOUT_H

#include "ch_extended.h"
#include "hal.h"
#include "type_traits_ex.h"
#include <utility>
#include <array>

namespace Digital {
  class OutputCommand
  {
  public:
    enum class Mode {
      Set,
      Clear,
      SetAndClear,
      Write,
      Toggle
    };
  private:
    static constexpr size_t busWidth_ = 16;
    using ex_value_t = Utils::SelectSize_t<busWidth_ * 2>;
    ex_value_t value_;
    Mode mode_;
  public:
    using value_type = Utils::SelectSize_t<busWidth_>;
    static constexpr size_t GetBusWidth()
    {
      return busWidth_;
    }
    std::pair<Mode, ex_value_t> Get() const
    {
      return {mode_, value_};
    }
    ex_value_t GetValue() const
    {
      return value_;
    }
    Mode GetMode() const
    {
      return mode_;
    }
    void SetValue(ex_value_t value)
    {
      value_ = value;
    }
    void SetMode(Mode mode)
    {
      mode_ = mode;
    }
    Rtos::Status Set(Mode mode, ex_value_t value)
    {
      if(mode != Mode::SetAndClear && value > Utils::NumberToMask_v<busWidth_>) {
        return Rtos::Status::Failure;
      }
      SetValue(value);
      SetMode(mode);
      return Rtos::Status::Success;
    }
  };

  class Output : Rtos::BaseStaticThread<256>
  {
  private:
    using pinmap_t = std::array<uint16_t, OutputCommand::GetBusWidth()>;
    using value_t = OutputCommand::value_type;
    static const SPIConfig spicfg_;
    static const pinmap_t pinMap_;
    SPIDriver* const SPID_;
    value_t mappedVal_, rawVal_;
    void main() override
    {
      using Mode = OutputCommand::Mode;
      while(true) {
        /* Waiting for a queued message then retrieving it.*/
        thread_t *tp = chMsgWait();
        OutputCommand& cmd = *reinterpret_cast<OutputCommand*>(chMsgGet(tp));
        value_t value = static_cast<value_t>(cmd.GetValue());
        switch(cmd.GetMode()) {
        case Mode::Set:
          rawVal_ |= value;
          break;
        case Mode::Clear:
          rawVal_ &= ~value;
          break;
        case Mode::SetAndClear:
          rawVal_ |= value & 0xFFFF;
          rawVal_ &= ~static_cast<value_t>(cmd.GetValue() >> OutputCommand::GetBusWidth());
          break;
        case Mode::Write:
          rawVal_ = value;
          break;
        case Mode::Toggle:
          rawVal_ ^= value;
          break;
        }
        cmd.SetValue(rawVal_);
        chMsgRelease(tp, MSG_OK);
        if(auto temp = Remap(rawVal_); mappedVal_ != temp) {
          mappedVal_ = temp;
          SpiSend();
        }
      }
    }
    void SpiSend()
    {
      spiSelect(SPID_);
      spiStartSend(SPID_, 1, &mappedVal_);
    }
    static uint16_t Remap(uint16_t val)
    {
      uint16_t result{};
      for(size_t i{}; i < pinMap_.size(); ++i) {
        result |= val & (1UL << i) ? pinMap_[i] : 0;
      }
      return result;
    }
  public:
    Output() : SPID_{&SPID2}, mappedVal_{}, rawVal_{}
    { }
    void Init()
    {
      palSetPadMode(GPIOB, 13, PAL_MODE_STM32_ALTERNATE_PUSHPULL);            // CLK
      palSetPadMode(spicfg_.ssport, spicfg_.sspad, PAL_MODE_OUTPUT_PUSHPULL); // RCLK
      palSetPadMode(GPIOB, 15, PAL_MODE_STM32_ALTERNATE_PUSHPULL);            // MOSI
      spiStart(SPID_, &spicfg_);
      start(NORMALPRIO);
    }
    msg_t SendMessage(OutputCommand& msg)
    {
      return chMsgSend(thread_ref, reinterpret_cast<msg_t>(&msg));
    }
    ~Output() override
    {
      spiStop(SPID_);
      palSetPadMode(GPIOB, 13, PAL_MODE_INPUT_PULLDOWN);                      // CLK
      palSetPadMode(spicfg_.ssport, spicfg_.sspad, PAL_MODE_INPUT_PULLDOWN);  // RCLK
      palSetPadMode(GPIOB, 15, PAL_MODE_INPUT_PULLDOWN);                      // MOSI
    }
  };

  extern Output output;
} //Digital

#endif // DIGITALOUT_H
