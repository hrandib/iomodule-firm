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

#include "analogin.h"
#include "type_traits_ex.h"

namespace Analog {

  Input input;

const ADCConversionGroup Input::adcGroupCfg_ {
  true, // is circular
  numChannels,
  AdcCb,
  nullptr,
  0, 0,           /* CR1, CR2 */
  0,
  Utils::Unpack3Bit(Utils::NumberToMask_v<numChannels>) * ADC_SAMPLE_28P5, /* SMPR2 */
  ADC_SQR1_NUM_CH(numChannels),
#if BOARD_VER == 1
  ADC_SQR2_SQ10_N(ADC_CHANNEL_IN9)| ADC_SQR2_SQ9_N(ADC_CHANNEL_IN8) |
  ADC_SQR2_SQ8_N(ADC_CHANNEL_IN4) | ADC_SQR2_SQ7_N(ADC_CHANNEL_IN7),
  ADC_SQR3_SQ6_N(ADC_CHANNEL_IN3) | ADC_SQR3_SQ5_N(ADC_CHANNEL_IN6) |
  ADC_SQR3_SQ4_N(ADC_CHANNEL_IN5) | ADC_SQR3_SQ3_N(ADC_CHANNEL_IN2) |
  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN0) | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN1)
#elif BOARD_VER == 2
  0,
  ADC_SQR3_SQ4_N(ADC_CHANNEL_IN1) | ADC_SQR3_SQ3_N(ADC_CHANNEL_IN2) |
  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN3) | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN4)
#endif
};

void Input::Init()
{
  InputPins::SetConfig<GpioModes::InputAnalog>();
  start(NORMALPRIO + 10);
  adcStart(&AdcDriver_, nullptr);
  adcStartConversion(&AdcDriver_, &adcGroupCfg_, (adcsample_t*)&dmaBuf_, dmaBufDepth);
}

void Input::AdcCb(ADCDriver* adcp, adcsample_t* buffer, size_t)
{
  Input& inp = *reinterpret_cast<Input*>(adcp->customData);
  sample_buf_t& sb = *reinterpret_cast<sample_buf_t*>(buffer);
  inp.fifo_.push(sb);
}

void Input::main()
{
  setName("AnalogInput");
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

} //Analog
