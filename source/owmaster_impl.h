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
#ifndef OWMASTER_IMPL_H
#define OWMASTER_IMPL_H

#include "ch_extended.h"
#include "at24_impl.h"
#include <atomic>

#define DS18B20_RESOLUTION_MASK 0x60
#define DS18B20_RESOLUTION_09BIT 0x00
#define DS18B20_RESOLUTION_10BIT 0x20
#define DS18B20_RESOLUTION_11BIT 0x40
#define DS18B20_RESOLUTION_12BIT 0x60

enum OWMasterCommand {
  owcmdNone,
  owcmdRescanNetwork,
  owcmdMeasurement,
  owcmdPrintOWList,
};

class OWMaster : Rtos::BaseStaticThread<512>
{
private:
  bool mesStarted;

  void Process();
  bool Process18B20GetTemp(int listPosition);
public:
  void Init();
  void main() override;

  void ExecNetScan();
  void ExecMeasurementCycle();
  msg_t SendMessage(OWMasterCommand msg);

  void Print();
};

extern OWMaster owMaster;

#endif // OWMASTER_IMPL_H
