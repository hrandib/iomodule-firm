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
#ifndef SDI_BASE_H
#define SDI_BASE_H

#include "ch_extended.h"
#include "hal.h"

#include <array>

namespace Sdi
{
  //without family code and crc
  using Id = std::array<uint8_t, 6>;

  enum Family {
    DS1992 = 0x08,
    DS1993 = 0x06,
    DS1996 = 0x0C,
    DS18B20 = 0x28
  };

  struct DeviceDescriptor;

  class SlaveBase : Rtos::BaseStaticThread<512>
  {
  public:
    using FullId_t = std::array<uint8_t, 8>;

  private:
    static const EXTConfig extcfg_;
    static const GPTConfig gptconf_;
    static void ExtCb(EXTDriver* extp, expchannel_t channel);
    static void GptCb(GPTDriver* gpt);

    std::array<uint8_t, 8> fullId_;
  public:
    SlaveBase() = default;
    SlaveBase(Family family, const Id& id)
    {
      fullId_[0] = static_cast<uint8_t>(family);
      std::copy(id.cbegin(), id.cend(), &fullId_[1]);
      FillCrc(fullId_);
    }
    void Init();
    void main() override;
    static void FillCrc(FullId_t& id)
    {

    }
  };

  extern SlaveBase sdi;

} //Sdi

#endif // SDI_BASE_H
