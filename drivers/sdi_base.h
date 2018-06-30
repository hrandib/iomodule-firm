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

  enum Timings { //in microseconds
    PeriodResetPulse = 480,
    PeriodPresenceWait = 20,
    PeriodPresencePulse = 90,
    PeriodBitSampling = 25,
    PeriodZeroPulse = 50,
    PeriodMaxTimeout = 0xFF00
  };

  enum class Command {
    NOP,
    SearchRom = 0xF0,
    ReadRom = 0x33,
    MatchRom = 0x55,
    SkipRom = 0xCC
  };

  class SlaveBase : Rtos::BaseStaticThread<512>
  {
  public:
    using FullId_t = std::array<uint8_t, 8>;

  private:
    enum class FSM {
      waitReset,
      reset,
      presenceWait,
      presenceStart,
      presenceEnd,
      readCommand,
      processCommand,
    };

    enum class From {
      Exti,
      Gpt
    };

    static const EXTConfig extcfg_;
    static const GPTConfig gptconf_;
    static void ExtCb(EXTDriver* extp, expchannel_t channel);
    static void GptCb(GPTDriver* gpt);

    EXTDriver* const EXTD_;
    GPTDriver* const GPTD_;
    std::array<uint8_t, 8> fullId_;
    uint32_t bitBuf_;
    uint8_t bitCount_;
    FSM fsm_;
    Command cmd_;

    void WriteOne();
    void WriteZero();
    void ReadBit();
    void ProcessCommand(From from);
    void SearchRom(From from);
  public:
    SlaveBase(Family family, const Id& id);
    void Init();
    void main() override;
    static void FillCrc(FullId_t& id);
    uint8_t GetCrc()
    {
      return fullId_.back();
    }
  };

} //Sdi

#endif // SDI_BASE_H
