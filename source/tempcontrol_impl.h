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
#ifndef TEMPCONTROL_IMPL_H
#define TEMPCONTROL_IMPL_H

#include "ch_extended.h"
#include "at24_impl.h"
#include <atomic>

typedef struct {
  uint8_t id[2][8];
  uint16_t temp[2];
} __attribute__((__packed__)) tcChannelConfig_t ;

enum TempControlCommand {
  tccmdNone,
  tccmdPrint,
};

class TempControl : Rtos::BaseStaticThread<512>
{
private:
  static const uint8_t MaxChannels = 4;
  tcChannelConfig_t channels[MaxChannels];

  void Process();
public:
  void Init();
  void main() override;

  bool SetID(uint8_t channel, uint8_t sensorn, uint8_t *id);
  bool SetTemp(uint8_t channel, uint8_t sensorn, uint16_t temperature);

  void Print(BaseSequentialStream *chp);

  msg_t SendMessage(TempControlCommand msg);
};

extern TempControl tempControl;

#endif // TEMPCONTROL_IMPL_H
