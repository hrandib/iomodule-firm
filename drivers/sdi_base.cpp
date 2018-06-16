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

enum Timings { //in microseconds
  ResetPulse = 480,
  PresenceWait = 25,
  PresencePulse = 100,
};

static enum class FSM {
  waitReset,
  reset,
  presenceWait,
  presenceStart,
  presenceEnd,

} fsm;

static void extcb1(EXTDriver* extp, expchannel_t channel) {
  static uint16_t counter;
  switch(fsm) {
    case FSM::waitReset:
    if(!palReadPad(RX_PORT, RX_PIN)) {
      fsm = FSM::reset;
      Rtos::SysLockGuardFromISR lock{};
      gptStartOneShotI(&GPTD4, 0xFFFF);
    }
    break;
  case FSM::reset:
    if(palReadPad(RX_PORT, RX_PIN)) {
      counter = (uint16_t)gptGetCounterX(&GPTD4);
      if(counter > 480) {
        fsm = FSM::presenceWait;
        Rtos::SysLockGuardFromISR lock{};
        gptStartOneShotI(&GPTD4, PresenceWait);
        extChannelDisableI(extp, channel);
        mbExti.postI((int32_t)counter);
      }
    }

  default:
    ;
  }
}


static void gptCb(GPTDriver* /*gpt*/)
{
  switch(fsm) {
  case FSM::presenceWait: {
    fsm = FSM::presenceStart;
    palClearPad(TX_PORT, TX_PIN);
    Rtos::SysLockGuardFromISR lock{};
    gptStartOneShotI(&GPTD4, PresencePulse);
  }
  break;
  case FSM::presenceStart: {
    fsm = FSM::presenceEnd;
    palSetPad(TX_PORT, TX_PIN);
    Rtos::SysLockGuardFromISR lock{};
    gptStartOneShotI(&GPTD4, 0xFFFF);
  }
  default:
    fsm = FSM::waitReset;
    Rtos::SysLockGuardFromISR lock{};
    extChannelEnableI(&EXTD1, RX_PIN);
  }
}

const GPTConfig gptconf{1000000, gptCb, 0, 0};

static const EXTConfig extcfg = {
  {
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOB, extcb1},
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
  gptStart(&GPTD4, &gptconf);
  extStart(&EXTD1, &extcfg);
  start(NORMALPRIO);
}

void SlaveBase::main() {
  int32_t val;
  while(true) {
    if(MSG_OK == mbExti.fetch(&val, 1)) {
      chprintf((BaseSequentialStream*)&SD1, "EXTI event: %u\r\n", val);
    }
  }
}

SlaveBase sdi;

} //Sdi

