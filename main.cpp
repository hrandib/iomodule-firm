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

#include "usbcfg.h"
#include "gpio.h"

using namespace Rtos;
using namespace Mcudrv;

using Led = Pc13;

int main(void) {
  halInit();
  System::init();
  Led::SetConfig<GpioBase::Out_PushPull>();
  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);
  usbStart(serusbcfg.usbp, &usbcfg);
  while(true) {
    Led::Toggle();
    BaseThread::sleep(MS2ST(100));
    chprintf((BaseSequentialStream*)&SDU1, "Hello!!\n");
  }
}
