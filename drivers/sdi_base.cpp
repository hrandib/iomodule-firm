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

//Device id w/o CRC8 part (56 bit)
const uint8_t sdiId[8] = {0x0C, 0x55, 0xAA, 0x33, 0x22, 0x11, 0x01};

namespace Sdi {

static Rtos::Mailbox<const char*, 16> mbExti;
static uint8_t bitCount;
static uint32_t bitBuf;

enum Timings { //in microseconds
  PeriodResetPulse = 480,
  PeriodPresenceWait = 20,
  PeriodPresencePulse = 90,
  PeriodBitSampling = 25,
  PeriodZeroPulse = 50,
  PeriodMaxTimeout = 0xFF00
};

static enum class FSM {
  waitReset,
  reset,
  presenceWait,
  presenceStart,
  presenceEnd,
  readCommand,
  processCommand,
} fsm;

static enum class Command {
  NOP,
  SearchRom = 0xF0,
  ReadRom = 0x33,
  MatchRom = 0x55,
  SkipRom = 0xCC
} cmd;

void Log(const char* msg)
{
  Rtos::SysLockGuardFromISR lock{};
  mbExti.postI(msg);
}

static void WriteOne()
{
  palClearPad(TX_PORT, TX_PIN);
}

static void WriteZero()
{
  palSetPad(TX_PORT, TX_PIN);
  Rtos::SysLockGuardFromISR lock{};
  extChannelDisableI(&EXTD1, RX_PIN);
  gptStopTimerI(&GPTD4);
  gptStartOneShotI(&GPTD4, PeriodZeroPulse);
}

static bool GetIdBit(size_t bitPos)
{
  return sdiId[bitPos >> 3] & (1 << (bitPos & 0x07));
}

static void ReadBit()
{
  Rtos::SysLockGuardFromISR lock{};
  gptStopTimerI(&GPTD4);
  gptStartOneShotI(&GPTD4, PeriodBitSampling);
}

enum class From {
  Exti,
  Gpt
};

static void SearchRom(From from)
{
  static enum State {
    WriteNormal,
    WriteComplement,
    ReadMaster,
    ResultMaster
  } state;
  if(from == From::Exti) {
    switch(state) {
    case WriteNormal:
      state = WriteComplement;
      if(!GetIdBit(bitCount)) {
        WriteZero();
      }
      break;
    case WriteComplement:
      state = ReadMaster;
      if(GetIdBit(bitCount)) {
        WriteZero();
      }
      break;
    case ReadMaster:
      state = ResultMaster;
      ReadBit();
      break;
    }
  }
  else if(state == ResultMaster) {
//    Log("Result Master");
    if(palReadPad(RX_PORT, RX_PIN) == GetIdBit(bitCount)) {
      state = WriteNormal;
      ++bitCount;
      if(bitCount == 64) {
        fsm = FSM::waitReset;
        bitCount = 0;
        bitBuf = 0;
//        Log("Bitcount");
      }
    }
    else {
      state = WriteNormal;
      fsm = FSM::waitReset;
      bitCount = 0;
      bitBuf = 0;
    }
  }
  else {
    WriteOne();
    Rtos::SysLockGuardFromISR lock{};
    extChannelEnableI(&EXTD1, RX_PIN);
  }
}

static void ProcessCommand(From from)
{
  switch(cmd) {
  case Command::SearchRom:
    SearchRom(from);
    break;
  default:
    Rtos::SysLockGuardFromISR lock{};
    mbExti.postI("Unknown command");
  }
}

static void extcb1(EXTDriver* extp, expchannel_t channel) {
  static uint16_t counter;
  switch(fsm) {
    case FSM::waitReset:
    if(!palReadPad(RX_PORT, RX_PIN)) {
      fsm = FSM::reset;
      Rtos::SysLockGuardFromISR lock{};
      gptStopTimerI(&GPTD4);
      gptStartOneShotI(&GPTD4, PeriodMaxTimeout);
//      mbExti.postI("Reset Start");
    }
    break;
  case FSM::reset:
    if(palReadPad(RX_PORT, RX_PIN)) {
      counter = (uint16_t)gptGetCounterX(&GPTD4);
      if(counter > PeriodResetPulse) {
        fsm = FSM::presenceWait;
        Rtos::SysLockGuardFromISR lock{};
        extChannelDisableI(extp, channel);
        gptStopTimerI(&GPTD4);
        gptStartOneShotI(&GPTD4, PeriodPresenceWait);
//        mbExti.postI("Reset End");
      }
      else {
        fsm = FSM::waitReset;
      }
    }
    break;
  case FSM::readCommand: {
    Rtos::SysLockGuardFromISR lock{};
    gptStopTimerI(&GPTD4);
    if(!palReadPad(RX_PORT, RX_PIN)) {
      gptStartOneShotI(&GPTD4, PeriodBitSampling);
      extChannelDisableI(extp, channel);
    }
    else {
      gptStartOneShotI(&GPTD4, PeriodMaxTimeout);
    }
  }
    break;
  case FSM::processCommand:
    if(!palReadPad(RX_PORT, RX_PIN)) {
      ProcessCommand(From::Exti);
    }
    break;
  default: {
    fsm = FSM::waitReset;
    Rtos::SysLockGuardFromISR lock{};
    mbExti.postI("EXTI callback default. FSM value:");
    mbExti.postI((char*)fsm);
    extChannelDisableI(extp, channel);
  }
  } //switch
}


static void gptCb(GPTDriver* gpt)
{
  switch(fsm) {
  case FSM::presenceWait: {
    fsm = FSM::presenceStart;
    palSetPad(TX_PORT, TX_PIN);
    Rtos::SysLockGuardFromISR lock{};
//    mbExti.postI("Presence Start");
    gptStartOneShotI(gpt, PeriodPresencePulse);
    }
    break;
  case FSM::presenceStart: {
    fsm = FSM::presenceEnd;
    palClearPad(TX_PORT, TX_PIN);
    Rtos::SysLockGuardFromISR lock{};
//    mbExti.postI("Presence End");
    gptStopTimerI(gpt);
    gptStartOneShotI(gpt, 80);
    }
    break;
  case FSM::presenceEnd: {
    fsm = FSM::readCommand;
    bitCount = 0;
    bitBuf = 0;
    Rtos::SysLockGuardFromISR lock{};
    extChannelEnableI(&EXTD1, RX_PIN);
//    mbExti.postI("Read command");
    }
    break;
  case FSM::readCommand:
    if((uint16_t)gptGetCounterX(gpt) >= PeriodMaxTimeout) {
      fsm = FSM::waitReset;
      Rtos::SysLockGuardFromISR lock{};
      mbExti.postI("Timeout");
    }
    if(bitCount < 7) {
      bitBuf |= palReadPad(RX_PORT, RX_PIN) << bitCount++;
    }
    else {
      bitBuf |= palReadPad(RX_PORT, RX_PIN) << bitCount;
      fsm = FSM::processCommand;
      cmd = Command(bitBuf);
      bitCount = 0;
      bitBuf = 0;
    } {
      Rtos::SysLockGuardFromISR lock{};
      extChannelEnableI(&EXTD1, RX_PIN);
    }
    break;
  case FSM::processCommand:
    ProcessCommand(From::Gpt);
    break;
  default: {
    Rtos::SysLockGuardFromISR lock{};
    extChannelEnableI(&EXTD1, RX_PIN);
    mbExti.postI("GPT callback default. FSM value:");
    mbExti.postI((char*)fsm);
    fsm = FSM::waitReset;
    }
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
  const char* msg;
  while(true) {
    if(MSG_OK == mbExti.fetch(&msg, 1)) {
      if((uint32_t)msg < 0x8000000U) {
        chprintf((BaseSequentialStream*)&SD1, "%d\r\n", uint32_t(msg));
      }
      else {
        chprintf((BaseSequentialStream*)&SD1, "%s\r\n", msg);
      }
    }
  }
}

SlaveBase sdi;

} //Sdi

