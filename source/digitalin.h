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
#ifndef DIGITALIN_H
#define DIGITALIN_H

#include "analogin.h"

namespace Digital {
using namespace Mcudrv;

  class Input : Rtos::BaseStaticThread<64>
  {
  public:
    static constexpr size_t numChannels = 4 + Analog::Input::numChannels;
  private:
    using Pins = Pinlist<Pc15, Pc14, Pc13, Pb2>;
    using counters_buf_t = std::array<uint32_t, numChannels>;
    using internal_counters_buf_t = std::array<uint32_t, 4>;
    GPTDriver& GPTD_;
    static const GPTConfig gptconf_;
    Rtos::BinarySemaphore semVal_, semCounters_;
    memory_relaxed_acquire_release::CircularFifo<uint32_t, 4> fifo_;
    internal_counters_buf_t counters_;
    uint16_t binaryVal_;
    static void gptCb(GPTDriver* gpt);
  public:
    Input() : GPTD_{GPTD2}, semVal_{false}, semCounters_{false}, counters_{}, binaryVal_{}
    {
      GPTD_.customData = this;
    }
    void Init()
    {
      Pins::SetConfig<GpioModes::InputFloating>();
      Pa12::SetConfig<GpioModes::OutputPushPull>();
      start(NORMALPRIO + 9);
      gptStart(&GPTD_, &gptconf_);
      gptStartContinuous(&GPTD_, 200); //500Hz
    }
    uint16_t GetBinaryVal() {
      uint16_t result = Analog::input.GetBinaryVal();
      result <<= 3;
      result = (result & ~(1U << 12)) | uint16_t((result & (1U << 12)) << 1);
      semVal_.wait();
      result |= binaryVal_;
      semVal_.signal();
      return result;
    }
    counters_buf_t GetCounters()
    {
      counters_buf_t result;
      semCounters_.wait();
      std::copy(counters_.begin(), counters_.end(), result.begin());
      semCounters_.signal();
      auto Din3Val = result[3];
      auto adcCounters = Analog::input.GetCounters();
      std::copy(adcCounters.begin(), adcCounters.end() - 1, &result[3]);
      result[12] = Din3Val;
      result[13] = adcCounters.back();
      return result;
    }

    void main() override
    {
      setName("DigitalInput");
      uint32_t previousVal;
      while(true) {
        uint32_t val;
        if(fifo_.pop(val) == false) {
          sleep(MS2ST(1));
          continue;
        }
        if(val != previousVal) {
          semCounters_.wait();
          for(size_t i = 0; i < numChannels; ++i) {
            if((val & (1U << i)) > (previousVal & (1U << i))) {
              ++counters_[i];
            }
          }
          semCounters_.signal();
          previousVal = val;
          //Din3 shifted according connector position
          val = (val & (uint32_t)~0b1000) | uint32_t((val & 0b1000) << 9);
          Rtos::SemLockGuard{semVal_};
          binaryVal_ = static_cast<uint16_t>(val);
        }
      }
    }
  };

  extern Input input;

} //Digital

#endif // DIGITALIN_H
