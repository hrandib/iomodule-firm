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
#include "crc8.h"

#define TX_PORT GPIOA
#define TX_PIN 12
#define RX_PORT GPIOB
#define RX_PIN 5

namespace Sdi {

static Rtos::Mailbox<const char*, 16> mbExti;

void Log(const char* msg)
{
  Rtos::SysLockGuardFromISR lock{};
  mbExti.postI(msg);
}

void SlaveBase::WriteOne()
{
  palClearPad(TX_PORT, TX_PIN);
}

void SlaveBase::WriteZero()
{
  palSetPad(TX_PORT, TX_PIN);
  Rtos::SysLockGuardFromISR lock{};
  extChannelDisableI(EXTD_, RX_PIN);
  gptStopTimerI(GPTD_);
  gptStartOneShotI(GPTD_, PeriodZeroPulse);
}

bool SlaveBase::GetIdBit(size_t bitPos)
{
  return fullId_[bitPos >> 3] & (1 << (bitPos & 0x07));
}

void SlaveBase::ReadBit()
{
  Rtos::SysLockGuardFromISR lock{};
  gptStopTimerI(GPTD_);
  gptStartOneShotI(GPTD_, PeriodBitSampling);
}

void SlaveBase::SearchRom(From from)
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
      if(!GetIdBit(bitCount_)) {
        WriteZero();
      }
      break;
    case WriteComplement:
      state = ReadMaster;
      if(GetIdBit(bitCount_)) {
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
    if(palReadPad(RX_PORT, RX_PIN) == GetIdBit(bitCount_)) {
      state = WriteNormal;
      ++bitCount_;
      if(bitCount_ == 64) {
        fsm_ = FSM::readCommand;
        bitCount_ = 0;
        bitBuf_ = 0;
      }
    }
    else {
      state = WriteNormal;
      fsm_ = FSM::waitReset;
      bitCount_ = 0;
      bitBuf_ = 0;
      Log("Excluded");
    }
  }
  else {
    WriteOne();
    Rtos::SysLockGuardFromISR lock{};
    extChannelEnableI(EXTD_, RX_PIN);
  }
}

void SlaveBase::MatchRom(From from)
{
  if(from == From::Exti) {
    ReadBit();
  }
  else if(palReadPad(RX_PORT, RX_PIN) == GetIdBit(bitCount_)) {
    ++bitCount_;
    if(bitCount_ == 64) {
      fsm_ = FSM::readCommand;
      bitCount_ = 0;
      bitBuf_ = 0;
    }
  }
  else {
    fsm_ = FSM::waitReset;
    bitCount_ = 0;
    bitBuf_ = 0;
    Log("Excluded");
  }
}

bool SlaveBase::FunctionCommandHandler(From from)
{
  Log((char*)cmd_);
  return false;
}

void SlaveBase::RomCommandHandler(From from)
{
  switch(cmd_) {
  case Command::NOP: case Command::ReadRom:
    break;
  case Command::SearchRom:
    SearchRom(from);
    break;
  case Command::MatchRom:
    MatchRom(from);
    break;
  case Command::SkipRom:
    fsm_ = FSM::readCommand;
    break;
  default:
    if(!FunctionCommandHandler(from)) {
      Rtos::SysLockGuardFromISR lock{};
      mbExti.postI("Unknown command");
    }
  }
}

void SlaveBase::ExtCb(EXTDriver* extp, expchannel_t channel) {
  static uint16_t counter;
  SlaveBase& sb = *static_cast<SlaveBase*>(extp->customData);
  auto& fsm = sb.fsm_;
  switch(fsm) {
    case FSM::waitReset:
    if(!palReadPad(RX_PORT, RX_PIN)) {
      fsm = FSM::reset;
      Rtos::SysLockGuardFromISR lock{};
      gptStopTimerI(sb.GPTD_);
      gptStartOneShotI(sb.GPTD_, PeriodMaxTimeout);
//      mbExti.postI("Reset Start");
    }
    break;
  case FSM::reset:
    if(palReadPad(RX_PORT, RX_PIN)) {
      counter = (uint16_t)gptGetCounterX(sb.GPTD_);
      if(counter > PeriodResetPulse) {
        fsm = FSM::presenceWait;
        Rtos::SysLockGuardFromISR lock{};
        extChannelDisableI(extp, channel);
        gptStopTimerI(sb.GPTD_);
        gptStartOneShotI(sb.GPTD_, PeriodPresenceWait);
//        mbExti.postI("Reset End");
      }
      else {
        fsm = FSM::waitReset;
      }
    }
    break;
  case FSM::readCommand: {
    Rtos::SysLockGuardFromISR lock{};
    gptStopTimerI(sb.GPTD_);
    if(!palReadPad(RX_PORT, RX_PIN)) {
      gptStartOneShotI(sb.GPTD_, PeriodBitSampling);
      extChannelDisableI(extp, channel);
    }
    else {
      gptStartOneShotI(sb.GPTD_, PeriodMaxTimeout);
    }
  }
    break;
  case FSM::processCommand:
    if(!palReadPad(RX_PORT, RX_PIN)) {
      sb.RomCommandHandler(From::Exti);
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

void SlaveBase::GptCb(GPTDriver* gpt)
{
  SlaveBase& sb = *static_cast<SlaveBase*>(gpt->customData);
  auto& fsm = sb.fsm_;
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
    sb.bitCount_ = 0;
    sb.bitBuf_ = 0;
    Rtos::SysLockGuardFromISR lock{};
    extChannelEnableI(sb.EXTD_, RX_PIN);
//    mbExti.postI("Read command");
    }
    break;
  case FSM::readCommand:
    if((uint16_t)gptGetCounterX(gpt) >= PeriodMaxTimeout) {
      fsm = FSM::waitReset;
      Rtos::SysLockGuardFromISR lock{};
      mbExti.postI("Timeout");
    }
    if(sb.bitCount_ < 7) {
      sb.bitBuf_ |= palReadPad(RX_PORT, RX_PIN) << sb.bitCount_++;
    }
    else {
      sb.bitBuf_ |= palReadPad(RX_PORT, RX_PIN) << sb.bitCount_;
      fsm = FSM::processCommand;
      sb.cmd_ = Command(sb.bitBuf_);
      sb.bitCount_ = 0;
      sb.bitBuf_ = 0;
    } {
      Rtos::SysLockGuardFromISR lock{};
      extChannelEnableI(sb.EXTD_, RX_PIN);
    }
    break;
  case FSM::processCommand:
    sb.RomCommandHandler(From::Gpt);
    break;
  default: {
    Rtos::SysLockGuardFromISR lock{};
    extChannelEnableI(sb.EXTD_, RX_PIN);
    mbExti.postI("GPT callback default. FSM value:");
    mbExti.postI((char*)gptGetCounterX(gpt));
    fsm = FSM::waitReset;
    }
  }
}

SlaveBase::SlaveBase(Family family, const Id& id) : EXTD_{&EXTD1}, GPTD_{&GPTD4}, bitBuf_{}, bitCount_{}, fsm_{}, cmd_{}
{
  EXTD_->customData = GPTD_->customData = static_cast<SlaveBase*>(this);
  palSetPadMode(TX_PORT, TX_PIN, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(RX_PORT, RX_PIN, PAL_MODE_INPUT);

  fullId_[0] = static_cast<uint8_t>(family);
  std::copy(id.cbegin(), id.cend(), &fullId_[1]);
  FillCrc(fullId_);
}

const GPTConfig SlaveBase::gptconf_{1000000, GptCb, 0, 0};

const EXTConfig SlaveBase::extcfg_{
  {
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_DISABLED, nullptr},
    {EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOB, ExtCb},
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
  gptStart(GPTD_, &gptconf_);
  extStart(EXTD_, &extcfg_);
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

void SlaveBase::FillCrc(SlaveBase::FullId_t& id)
{
  Crc::Crc8_NoLUT crc{};
  id.back() = crc(id.data(), (uint8_t)id.size() - 1).GetResult();
}


} //Sdi

