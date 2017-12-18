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

#include "wake_base.h"
#include "display.h"
#include "led_driver.h"
#include "button_control.h"
#include "measure.h"

using namespace Rtos;

using LedDriver = Wk::LedDriver<>;

using ButtonControl = Wk::ButtonControl<LedDriver>;

static Wk::Wake<LedDriver> wake(UARTD1, 9600, GPIOA, 10);

static Display disp;
static Measure meas;

int main(void) {

  halInit();
  System::init();
  using namespace Mcudrv;
  GpioB::Enable();
  wake.Init();
  disp.Init();
  meas.Init();
  ButtonControl buttonControl{GPIOB, 10};
  while (true) {
    buttonControl.Update();
    BaseThread::sleep(buttonControl.GetUpdatePeriod());
  }
}
