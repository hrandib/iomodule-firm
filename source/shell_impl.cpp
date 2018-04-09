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

#include "hal.h"
#include "shell.h"
#include "shell_impl.h"
#include "analogout.h"
#include "chprintf.h"
#include <cstdlib>

static THD_WORKING_AREA(SHELL_WA_SIZE, 512);

static void cmd_setanalog(BaseSequentialStream *chp, int argc, char *argv[]);

static const ShellCommand commands[] = {
  {"setanalog", cmd_setanalog},
  {nullptr, nullptr}
};

static char histbuf[128];

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SD1,
  commands,
  histbuf,
  128
};

void cmd_setanalog(BaseSequentialStream *chp, int argc, char *argv[])
{
  if(argc == 2) do {
    pwmchannel_t ch = *argv[0] - '0';
    if(ch > 3) {
      break;
    }
    int value = atoi(argv[1]);
    if(value < 0 || value > 4096) {
      break;
    }
    Analog::OutputCommand cmd{};
    cmd.SetValue(ch, (uint16_t)value);
    Analog::output.SendMessage(cmd);
    chprintf(chp, "%4u %4u %4u %4u\r\n",
                  cmd.GetValue(0),
                  cmd.GetValue(1),
                  cmd.GetValue(2),
                  cmd.GetValue(3));
    return;
  } while(false);
  shellUsage(chp, "Set analog output value on selected channel"
                  "\r\nReturns array of current values"
                  "\r\n\tsetanalog [channel(0-3)] [value(0-4096)]"
                  "\r\nExample:"
                  "\r\n\tsetanalog 1 2048");
}

Shell::Shell()
{
  palSetPadMode(GPIOB, 6, PAL_MODE_STM32_ALTERNATE_PUSHPULL); // tx
  palSetPadMode(GPIOB, 7, PAL_MODE_INPUT); //rx
  SerialConfig sercfg{ 115200,
                       USART_CR1_TE | USART_CR1_RE | USART_CR1_UE,
                           0,
                           0};
  sdStart(&SD1, &sercfg);
  shellInit();
  chThdCreateStatic(SHELL_WA_SIZE, sizeof(SHELL_WA_SIZE), NORMALPRIO, shellThread, (void*)&shell_cfg1);
}
