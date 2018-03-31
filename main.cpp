/*
 * Copyright (c) 2017 Dmytro Shestakov
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

#include <stdio.h>
#include <string.h>
#include <cstdlib>

#include "ch_extended.h"
#include "hal.h"
#include "chprintf.h"

#include "pinlist.h"

using namespace Rtos;
using namespace Mcudrv;

using Outputs_A = Pinlist<Pa8, SequenceOf<4>>;
using Outputs_B = Pinlist<Pb6, SequenceOf<2>>;
using Outputs_C = Pinlist<Pc13, SequenceOf<3>>;

template<typename DataPin, typename ClkPin, typename LatchPin, uint8_t SizeInBytes = 1>
class SerialReg
{
private:
    enum { SizeInBits = SizeInBytes * 8 };
    typedef typename Utils::SelectSize<SizeInBits>::type DataType;
public:
    static void Init()
    {
        DataPin::template SetConfig<GpioBase::Out_PushPull_fast>();
        ClkPin::template SetConfig<GpioBase::Out_PushPull_fast>();
        LatchPin::template SetConfig<GpioBase::Out_PushPull_fast>();
    }
    static void Write(DataType data, bool NoLatch = false)
    {
        for(uint8_t i = 0; i < SizeInBits; ++i) {
            DataPin::SetOrClear(data & (1UL << (SizeInBits - 1)));
            ClkPin::Set();
            data <<= 1;
            ClkPin::Clear();
        }
        if(!NoLatch) {
            Latch();
        }
    }
    static void Latch()
    {
        LatchPin::Set();
        __NOP();
        __NOP();
        LatchPin::Clear();
    }

};

using SR = SerialReg<Pb15, Pb13, Pb14, 2>;

int main(void) {
  halInit();
  System::init();
  SR::Init();
  size_t counter = 0;
  while(true) {
    if(counter > 15) {
      counter = 0;
    }
    SR::Write(uint16_t(1 << counter++));
    BaseThread::sleep(S2ST(1));
  }
}
