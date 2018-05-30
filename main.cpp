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
#include "analogin.h"
#include "digitalin.h"
#include "digitalout.h"
#include "modbus_impl.h"
#include "at24_impl.h"

using namespace Rtos;
using namespace Mcudrv;

using nvram::File;

static constexpr auto& dout = Digital::output;
static constexpr auto& aout = Analog::output;
static constexpr auto& ain = Analog::input;
static constexpr auto& din = Digital::input;

static auto Init = [](auto&&... objs) {
  (objs.Init(), ...);
};

std::atomic_uint32_t uptimeCounter;

const uint8_t test_string[] = "123";
static uint8_t read_back_buf[4];

static const I2CConfig i2cfg1 = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2
};

int main(void) {
  halInit();
  System::init();
  palSetPadMode(GPIOB, 8, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);   /* SCL */
  palSetPadMode(GPIOB, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);   /* SDA */
//  Init(aout, dout, ain, din, modbus);
  Shell sh;
  /* normal test and simple example of usage */
  i2cStart(&I2CD1, &i2cfg1);
  NvramInit();
  File *f = NvramTryOpen("test0", 4);
  osalDbgCheck(nullptr != f);
  fileoffset_t status;

  status = f->write(test_string, sizeof(test_string));
  osalDbgCheck(sizeof(test_string) == status);

  status = f->setPosition(0);
  osalDbgCheck(FILE_OK == status);

  status = f->read(read_back_buf, sizeof(test_string));
  chprintf((BaseSequentialStream*)&SD1, (const char*)read_back_buf);
  osalDbgCheck(sizeof(test_string) == status);
  systime_t time = chVTGetSystemTimeX();
  while(true) {
    time += S2ST(1);
    BaseThread::sleepUntil(time);
    ++uptimeCounter;
  }
}
