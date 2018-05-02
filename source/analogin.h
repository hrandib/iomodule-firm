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
    using counters_buf_t = std::array<uint32_t, numChannels>;
    using InputPins = Pinlist<Pinlist<Pa0, SequenceOf<8>>, Pinlist<Pb0, SequenceOf<2>>>;

    dma_buf_t dmaBuf_;
    fifo_t fifo_;
    std::array<MovingAverageBuf<adcsample_t, 8>, numChannels> maBuf_;
    ADCDriver& AdcDriver_;
    Rtos::BinarySemaphore semSamples_, semCounters_, semBinaryVal_;
    sample_buf_t samples_;
    counters_buf_t counters_;
    uint16_t binaryVal_;
    static const ADCConversionGroup adcGroupCfg_;
  public:
    Input() : fifo_{}, maBuf_{}, AdcDriver_{ADCD1},
      semSamples_{false}, semCounters_{false}, semBinaryVal_{false},
      samples_{}, counters_{}, binaryVal_{}
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
    sample_buf_t GetSamples()
    {
      Rtos::SemLockGuard{semSamples_};
      return samples_;
    }
    uint16_t GetDigitalVal()
    {
      Rtos::SemLockGuard{semBinaryVal_};
      return binaryVal_;
    }
    counters_buf_t GetCounters()
    {
      Rtos::SemLockGuard{semCounters_};
      return counters_;
    }
    void main() override
    {
      sample_buf_t buf;
      size_t AdcRefreshCount{};
      while(true) {
        if(fifo_.pop(buf) == false) {
          sleep(US2ST(500));
          continue;
        }
        uint16_t binarySet{}, binaryClear{};
        for(size_t i{}; i < numChannels; ++i) {
          maBuf_[i] = buf[i];
          buf[i] = maBuf_[i];
          if(buf[i] > highLevelThd) {
            binarySet |= (1U << i);
          }
          if(buf[i] < lowLevelThd) {
            binaryClear |= (1U << i);
          }
        }
        uint16_t positiveTransitionMask = ((binaryVal_ ^ binarySet) & ~binaryVal_);
        if(positiveTransitionMask) {
          Rtos::SemLockGuard{semCounters_};
          for(size_t i{}; i < numChannels; ++i) {
              counters_[i] += (positiveTransitionMask >> i) & 0x01;
          }
        }
        uint16_t binaryTemp = (binaryVal_ | binarySet) & ~binaryClear;
        if(binaryTemp != binaryVal_) {
          Rtos::SemLockGuard{semBinaryVal_};
          binaryVal_ = binaryTemp;
        }
        if(++AdcRefreshCount == 20) {
          AdcRefreshCount = 0;
          semSamples_.wait();
          samples_ = buf;
          semSamples_.signal();
        }
      }
    }

  };

  extern Input input;
} //Analog

#endif // ANALOGIN_H
