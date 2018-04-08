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
#include "type_traits_ex.h"

namespace Analog {

  class OutputCommand
  {
  private:
    static constexpr size_t chNumber_ = 4;
    static constexpr size_t maxMask_ = Utils::NumberToMask_v<chNumber_>;
    std::array<uint16_t, 4> values_;
    uint16_t channelMask_;
  public:
    constexpr size_t GetChannelNumber()
    {
      return chNumber_;
    }
    uint16_t GetChannelMask()
    {
      return channelMask_;
    }
    void SetChannelMask(uint16_t mask)
    {
      if(mask > maxMask_) {
        return;
      }
      channelMask_ = mask;
    }
    uint16_t GetValue(size_t ch)
    {
      return values_[ch];
    }
    void SetValue(size_t ch, uint16_t value)
    {
      if(ch > chNumber_) {
        return;
      }
      values_[ch] = value;
    }
  };

  class Output
  {

  };
}

#endif // ANALOGOUT_H
