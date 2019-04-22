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
#ifndef EXECUTOR_IMPL_H
#define EXECUTOR_IMPL_H

#include "ch_extended.h"
#include "at24_impl.h"
#include <atomic>

extern std::atomic_uint32_t uptimeCounter;

class Executor : Rtos::BaseStaticThread<512>
{
private:
  uint16_t oldRegBuffer16;
  void Process();

  bool TriacsDisabled;
  systime_t pinsTime[5] = {0};
  systime_t triacsTimeOn[4] = {0};
  systime_t triacsTimeOff[4] = {0};
  systime_t pinGOTime = 0; // impulse on small relay to make GlobalOff command on all controllers

  systime_t PINTimeOffSetup[16] = {0};
  systime_t PINTimeOff[16] = {0};

  bool IOSet(uint16_t reg);
  bool IOClear(uint16_t reg);
  bool IOToggle(uint16_t reg);
public:
  void Init();
  void main() override;

  void SetTriacsDisabled(bool state);
  bool GetTriacsDisabled();
  void SetPinOff(uint8_t channel, uint16_t time);

  bool OutSet(uint8_t channel, bool value);
  bool OutToggle(uint8_t channel);
  bool OutClearAll();
  bool OutGlobalOff();

  void Print();
};

extern Executor executor;

#endif // EXECUTOR_IMPL_H
