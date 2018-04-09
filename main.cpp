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
#include "pinlist.h"
#include "shell_impl.h"
#include "analogout.h"

using namespace Rtos;
using namespace Mcudrv;

static Analog::Output out;

int main(void) {
  halInit();
  System::init();
  Shell sh;
  out.Init();
  uint16_t i{}, val{};
  Analog::OutputCommand cmd{};
  while(true) {
    val += 30;
    if(val > 4095) {
      val = 0;
      if(++i == Analog::OutputCommand::GetChannelNumber()) {
        i = 0;
      }
    }
    cmd.SetValue(0, val);
    cmd.SetValue(1, val);
    cmd.SetValue(2, val);
    cmd.SetValue(3, val);
    out.SendMessage(cmd);
    BaseThread::sleep(S2ST(1));
  }
}
