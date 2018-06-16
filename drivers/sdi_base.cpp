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

#include "sdi_base.h"
#include "chprintf.h"

#define TX_PORT GPIOA
#define TX_PIN 12
#define RX_PORT GPIOB
#define RX_PIN 5

namespace Sdi {

static Rtos::Mailbox<int32_t, 8> mbExti;

static void extcb1(EXTDriver*, expchannel_t channel) {
  Rtos::SysLockGuardFromISR lock{};
  mbExti.postI((int32_t)channel);
}

static const EXTConfig extcfg = {
  {
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, extcb1},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr}
  }
};

void SlaveBase::Init()
{
  palSetPadMode(TX_PORT, TX_PIN, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(RX_PORT, RX_PIN, PAL_MODE_INPUT);
  extStart(&EXTD1, &extcfg);
  start(NORMALPRIO);
}

void SlaveBase::main() {
  static constexpr systime_t period = S2ST(3);
  int32_t val;
  systime_t time = Rtos::System::getTime() + period;
  while(true) {
    if(MSG_OK == mbExti.fetch(&val, 1)) {
      chprintf((BaseSequentialStream*)&SD1, "EXTI event: %x\r\n", val);
    }
    if(auto now = Rtos::System::getTime(); now >= time) {
      time += period;
      palTogglePad(TX_PORT, TX_PIN);
    }
  }
}

SlaveBase sdi;

} //Sdi

