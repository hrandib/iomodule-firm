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
#include "pinlist.h"
#include <utility>
#include <array>

namespace Digital {

  using namespace Mcudrv;

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
    static constexpr size_t busWidth = 9;
    using ex_value_t = Utils::SelectSize_t<busWidth * 2>;
    ex_value_t value_;
    Mode mode_;
  public:
    using value_type = Utils::SelectSize_t<busWidth>;
    static constexpr size_t GetBusWidth()
    {
      return busWidth;
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
      if(mode != Mode::SetAndClear && value > Utils::NumberToMask_v<busWidth>) {
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
    using Pins = Pinlist<Pa5, Pa6, Pa7, Pb0, Pb1, Pb2, Pb13, Pb14, Pb15>;
    using value_t = OutputCommand::value_type;
    value_t prevVal_, curVal_;
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
          curVal_ |= value;
          break;
        case Mode::Clear:
          curVal_ &= ~value;
          break;
        case Mode::SetAndClear:
          curVal_ |= value & Utils::NumberToMask_v<OutputCommand::GetBusWidth()>;
          curVal_ &= ~static_cast<value_t>(cmd.GetValue() >> OutputCommand::GetBusWidth());
          break;
        case Mode::Write:
          curVal_ = value;
          break;
        case Mode::Toggle:
          curVal_ ^= value;
          break;
        }
        cmd.SetValue(curVal_);
        chMsgRelease(tp, MSG_OK);
        if(prevVal_ != curVal_) {
          prevVal_ = curVal_;
          Pins::Write(curVal_);
        }
      }
    }
  public:
    Output() : prevVal_{}, curVal_{}
    { }
    void Init()
    {
      Pins::SetConfig<GpioModes::OutputPushPull>();
      start(NORMALPRIO);
    }
    uint16_t GetBinaryVal()
    {
      return (Pins::Read() & 0xffff);
    }
    msg_t SendMessage(OutputCommand& msg)
    {
      return chMsgSend(thread_ref, reinterpret_cast<msg_t>(&msg));
    }
  };

  extern Output output;
} //Digital

#endif // DIGITALOUT_H
