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
#ifndef ANALOGIN_H
#define ANALOGIN_H

#include "hal.h"
#include "chprintf.h"
#include "ch_extended.h"
#include "pinlist.h"
#include "type_traits_ex.h"
#include "circularfifo.h"

#include <array>
#include <numeric>

namespace Analog {
using namespace Mcudrv;

  template<typename T, size_t size>
  class MovingAverageBuf
  {
  private:
    std::array<T, size> buf_{};
    size_t index{};
  public:
    void operator=(T val)
    {
      buf_[index] = val;
      if constexpr(Utils::IsPowerOf2(size)) {
        index = (index + 1) & (size - 1);
      }
      else {
        index = (index + 1) % size;
      }
    }
    operator T()
    {
      return std::accumulate(begin(buf_), end(buf_), (adcsample_t)0) / size;
    }
  };

  class Input : Rtos::BaseStaticThread<256>
  {
  public:
    static constexpr size_t numChannels = 10;
    static constexpr size_t dmaBufDepth = 2;

    static constexpr size_t lowLevelThd = 4096 / 4;
    static constexpr size_t highLevelThd = 4096 / 2;
  private:
    using sample_buf_t = std::array<adcsample_t, numChannels>;
    using dma_buf_t = std::array<sample_buf_t, dmaBufDepth>;
    using fifo_t = memory_relaxed_aquire_release::CircularFifo<sample_buf_t, 64>;
    using InputPins = Pinlist<Pinlist<Pa0, SequenceOf<8>>, Pinlist<Pb0, SequenceOf<2>>>;

    dma_buf_t dmaBuf_;
    fifo_t fifo_;
    std::array<MovingAverageBuf<adcsample_t, 8>, numChannels> maBuf_;
    ADCDriver& AdcDriver_;
    std::array<uint32_t, numChannels> counters_;
    uint16_t digitalVal_;
    static const ADCConversionGroup adcGroupCfg_;
  public:
    Input() : fifo_{}, maBuf_{}, AdcDriver_{ADCD1}, counters_{}, digitalVal_{}
    {
      AdcDriver_.customData = this;
    }
    void Init()
    {
      InputPins::SetConfig<GpioModes::InputAnalog>();
      setName("AnalogInput");
      start(HIGHPRIO);
      adcStart(&AdcDriver_, nullptr);
      adcStartConversion(&AdcDriver_, &adcGroupCfg_, (adcsample_t*)&dmaBuf_, dmaBufDepth);
    }
    static void AdcCb(ADCDriver* adcp, adcsample_t* buffer, size_t /*n*/)
    {
      Input& inp = *reinterpret_cast<Input*>(adcp->customData);
      sample_buf_t& sb = *reinterpret_cast<sample_buf_t*>(buffer);
      inp.fifo_.push(sb);
    }
    void main() override
    {
      sample_buf_t buf;
      size_t counter{};
      while(true) {
        if(fifo_.pop(buf) == false) {
          sleep(MS2ST(1));
          continue;
        }
        for(size_t i; i < numChannels; ++i) {
          maBuf_[i] = buf[i];
          buf[i] = maBuf_[i];
          if(buf[i] > highLevelThd) {
            if(!(digitalVal_ & (1 << i))) {
              ++counters_[i];
            }
            digitalVal_ |= (1U << i);
          }
          if(buf[i] < lowLevelThd) {
            digitalVal_ &= ~(1U << i);
          }
        }
        if(++counter == 10000) {
          counter = 0;
          chprintf((BaseSequentialStream*)&SD1, "%u %u %u %u %u %u %u %u %u %u\r\n",
                          buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9]);
        }
      }
    }

  };

  extern Input input;
} //Analog

#endif // ANALOGIN_H
