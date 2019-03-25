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

#include "executor_impl.h"
#include "digitalin.h"
#include "digitalout.h"
#include "analogin.h"
#include "order_conv.h"
#include "chprintf.h"
#include "sconfig.h"

#if BOARD_VER == 1
#include "analogout.h"
#endif

#include <array>

using Utils::htons;
using Utils::ntohs;
using Utils::htonl;

Executor executor;

extern "C" {

static systime_t pinsTime[5] = {0};

void Executor::Process()
{
  uint16_t regBuffer16 = Digital::input.GetBinaryVal();
  uint16_t changedBuffer = oldRegBuffer16 ^ regBuffer16;
//  chprintf((BaseSequentialStream*)&SD1, "%x\r\n", regBuffer16);
  systime_t time = chVTGetSystemTimeX();

  for (uint8_t inputN = 0; inputN < 5; inputN++) {
    uint16_t vbit = uint16_t(1 << inputN);
    if ((time - pinsTime[inputN] > 200) &&
        (changedBuffer & vbit) &&
        (regBuffer16 & vbit) )
    {
      if (inputN > 3) {
        Digital::OutputCommand cmd{};
        cmd.Set(decltype(cmd)::Mode::Clear, 0x1F);
        Digital::output.SendMessage(cmd);
      } else {
        Digital::OutputCommand cmd{};
        cmd.Set(decltype(cmd)::Mode::Toggle, vbit * vbit);
        Digital::output.SendMessage(cmd);
      }

      pinsTime[inputN] = chVTGetSystemTimeX();
    }
  }

  oldRegBuffer16 = regBuffer16;
  return;
}

void Executor::Init()
{
  oldRegBuffer16 = 0xff;
  start(NORMALPRIO + 12);
}

void Executor::main()
{
  setName("Executor");
  sleep(MS2ST(3000));

  while(true) {
    if (Util::sConfig.GetExecutorEnable())
      Process();
    sleep(MS2ST(100));
  }
}

} // extern C
