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
    uint32_t value_;
    Mode mode_;
  public:
    static constexpr size_t GetBusWidth()
    {
      return busWidth_;
    }
    std::pair<Mode, uint32_t> Get() const
    {
      return {mode_, value_};
    }
    uint32_t GetValue() const
    {
      return value_;
    }
    Mode GetMode() const
    {
      return mode_;
    }
    void SetValue(uint32_t value)
    {
      value_ = value;
    }
    void SetMode(Mode mode)
    {
      mode_ = mode;
    }
    Rtos::Status Set(Mode mode, uint32_t value)
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
    static const SPIConfig spicfg_;
    SPIDriver* const SPID_;
    const IOBus spiBus_{GPIOA, 0x0F, 8};
    void main() override;
    void SetValue(pwmchannel_t ch, pwmcnt_t value);
  public:
    Output() : SPID_{&SPID2}
    { }
    void Init();
    msg_t SendMessage(OutputCommand& msg);
    ~Output() override;
  };
} //Digital

#endif // DIGITALOUT_H
