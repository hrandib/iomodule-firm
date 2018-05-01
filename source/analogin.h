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
#include "circularfifo.h"

#include <array>

namespace Analog {
using namespace Mcudrv;

  class Input : Rtos::BaseStaticThread<256>
  {
  public:
    static constexpr size_t numChannels = 10;
    static constexpr size_t bufDepth = 2;
    using sample_buf_t = std::array<adcsample_t, numChannels>;
    using dma_buf_t = std::array<sample_buf_t, bufDepth>;
    using fifo_t = memory_relaxed_aquire_release::CircularFifo<sample_buf_t, 128>;
  private:
    using InputPins = Pinlist<Pinlist<Pa0, SequenceOf<8>>, Pinlist<Pb0, SequenceOf<2>>>;
    dma_buf_t dmaBuf_;
    fifo_t fifo_;
    ADCDriver& AdcDriver_;
    static const ADCConversionGroup adcGroupCfg_;
  public:
    Input() : fifo_{}, AdcDriver_{ADCD1}
    {
      AdcDriver_.customData = this;
    }
    void Init()
    {
      InputPins::SetConfig<GpioModes::InputAnalog>();
      start(NORMALPRIO);
      adcStart(&AdcDriver_, nullptr);
      adcStartConversion(&AdcDriver_, &adcGroupCfg_, (adcsample_t*)&dmaBuf_, bufDepth);
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
