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

#include "tempcontrol_impl.h"
#include "chprintf.h"
#include <string.h>
#include "digitalin.h"
#include "digitalout.h"
#include "order_conv.h"
#include "chprintf.h"
#include "sconfig.h"
#include "onewire.h"
#include "chmsg.h"
#include "utils.h"
#include "crc8.h"

#if BOARD_VER == 1
#include "analogout.h"
#endif

#include <array>

using Utils::htons;
using Utils::ntohs;
using Utils::htonl;

TempControl tempControl;

extern "C" {

static const uint8_t idzero[8] = {0};

bool TempControl::ControlChannel(uint8_t channel, bool value) {
#if BOARD_VER == 1
  uint32_t chmask = 1 << channel;
#else
  uint32_t chmask = 1 << (channel * 2);
#endif

  Digital::OutputCommand cmd{};

  if (value)
    cmd.Set(decltype(cmd)::Mode::Set, chmask);
  else
    cmd.Set(decltype(cmd)::Mode::Clear, chmask);

  Digital::output.SendMessage(cmd);

  return true;
}

#define ifbit(b, n)((b) ? (1 << n) : 0x00)

void TempControl::Process()
{
  if (!OWire::owDriver.Ready())
    return;

  uint16_t regBuffer16 = Digital::output.GetBinaryVal();

  for (uint8_t ch = 0; ch < MaxChannels; ch ++) {

    // 1 - relay on 0 - relay off
#if BOARD_VER == 1
    bool chON = (regBuffer16 & (1 << ch));
#else
    bool chOn = (regBuffer16 & (1 << (ch * 2)));
#endif

    if (GetChEnabled(ch)) {
      uint16_t t1 = OWire::owDriver.getOwList()->GetTemperature(channels[ch].id[0]);
      uint16_t t2 = OWire::owDriver.getOwList()->GetTemperature(channels[ch].id[1]);
      // from -60C (4000) to +100C (20000)
      bool t1ok = t1 > 4000 && t1 < 20000;
      bool t2ok = t2 > 4000 && t2 < 20000;

      // save status
      chStatus[ch].state = 0;
      chStatus[ch].temp[0] = t1;
      chStatus[ch].temp[1] = t2;

      chStatus[ch].state |= ifbit(t1ok, 0);
      chStatus[ch].state |= ifbit(t2ok, 1);

      uint16_t cht1 = channels[ch].temp[0];
      uint16_t cht2 = channels[ch].temp[1];
      // from -60C (4000) to +100C (20000)
      bool cht1ok = cht1 > 4000 && cht1 < 20000;
      bool cht2ok = cht2 > 4000 && cht2 < 20000;

      chStatus[ch].state |= ifbit(cht1ok, 2);
      chStatus[ch].state |= ifbit(cht2ok, 3);

      bool desChOn = false;

      // safety check
      if (!cht1ok || !t1ok) {
        chStatus[ch].state |= ifbit(true, 4);
        if (chOn)
          ControlChannel(ch, false);
        continue;
      }

      // main long loop control
      if (cht2ok && t2ok && t2 + 100 < cht2) { // todo: add histeresis
        chStatus[ch].state |= ifbit(true, 5);
        desChOn = true;
      }

      // main short loop control
      if ((!cht2ok || !t2ok) && cht1ok && t1ok && t1 + 100 < cht1) { // todo: add histeresis
        chStatus[ch].state |= ifbit(true, 6);
        desChOn = true;
      }

      // check if long loop is overheated
      if (cht2ok && t2ok && t2 > cht2) {
        chStatus[ch].state |= ifbit(true, 7);
        desChOn = false;
      }

      // check if short loop is overheated
      if (cht1ok && t1ok && t1 > cht1) {
        chStatus[ch].state |= ifbit(true, 8);
        desChOn = false;
      }

      chStatus[ch].state |= ifbit(desChOn, 9);
      chStatus[ch].state |= ifbit(chOn, 10);

      // control
      if (chOn != desChOn) {
        chprintf((BaseSequentialStream*)&SD1, "[%d] t1 %d ch1 %d t2 %d ch2 %d state %04x\r\n", chVTGetSystemTimeX()/1000, t1, cht1, t2, cht2, chStatus[ch].state);
        chprintf((BaseSequentialStream*)&SD1, "ch %d current: %s control: %s\r\n", ch, chOn ? "on" : "off", desChOn ? "on" : "off");

        ControlChannel(ch, desChOn);
      }
    } else {
      // safety. disabled channel have to be in off state
      if (chOn) {
        ControlChannel(ch, false);

        chStatus[ch].state = 0x0000;
        chStatus[ch].temp[0] = 0xffff;
        chStatus[ch].temp[1] = 0xffff;
      }
    }
  }

  return;
}

void TempControl::Init() {
  for (int i = 0; i < MaxChannels; i++) {
    memset(channels[i].id[0], 0, 8);
    memset(channels[i].id[1], 0, 8);
    channels[i].temp[0] = 0xffff;
    channels[i].temp[1] = 0xffff;

    chStatus[i].state = 0x0000;
    chStatus[i].temp[0] = 0xffff;
    chStatus[i].temp[1] = 0xffff;
  }

  LoadFromEEPROM();

  start(NORMALPRIO + 15);
}

void TempControl::main() {
  setName("TempControl");
  sleep(MS2ST(300));

  while(true) {
    if (Util::sConfig.GetTempControlEnable())
      Process();

    /* Searching for a queued message then retrieving it.*/
    chSysLock();
    bool haveMsg = chMsgIsPendingI(currp);
    chSysUnlock();
    if (haveMsg) {
      thread_t *tp = chMsgWait();
       TempControlCommand *cmd = reinterpret_cast<TempControlCommand*>(chMsgGet(tp));

       switch (*cmd) {
       default:
         break;
       case tccmdPrint:
         Print((BaseSequentialStream*)&SD1);
         break;
       case tccmdPrintStatus:
         PrintStatus((BaseSequentialStream*)&SD1);
         break;
       case tccmdCfgLoadFromEEPROM:
         LoadFromEEPROM();
         break;
       case tccmdCfgSaveToEEPROM:
         SaveToEEPROM();
         break;
       }

      chMsgRelease(tp, MSG_OK);
    }

    sleep(MS2ST(300));
  }
}

bool TempControl::SetChEnable(uint8_t channel, bool en) {
  if (channel > 3)
    return false;

  if (en)
    settings = settings | (uint8_t)(1 << channel);
  else
    settings = settings & (0xff ^ (uint8_t)(1 << channel));

  return true;
}

bool TempControl::GetChEnabled(uint8_t channel) {
  if (channel > 3)
    return false;

  return (settings & (uint8_t)(1 << channel));
}

bool TempControl::SetID(uint8_t channel, uint8_t sensorn, uint8_t *id) {
  if (channel > 3 || sensorn > 1 || !id)
    return false;

  memcpy(&channels[channel].id[sensorn], id, 8);
  return true;
}

bool TempControl::SetTemp(uint8_t channel, uint8_t sensorn, uint16_t temperature) {
  if (channel > 3 || sensorn > 1)
    return false;

  channels[channel].temp[sensorn] = temperature;
  return true;
}

bool TempControl::SaveToEEPROM() {
  uint8_t data[sizeof(channels) + sizeof(settings) + 1];
  memcpy(&data, &channels, sizeof(channels));
  memcpy(&data[sizeof(channels)], &settings, sizeof(settings));
  data[sizeof(data) - 1] = crc8_ow(data, sizeof(data) - 1);

  if(sizeof(data) != nvram::eeprom.Write(nvram::Section::TempSetup, data)) {
    return false;
  }

  return true;
}

bool TempControl::LoadFromEEPROM() {
  uint8_t data[sizeof(channels) + sizeof(settings) + 1];
  size_t len = nvram::eeprom.Read(nvram::Section::TempSetup, data);
  if(sizeof(data) != len) {
    return false;
  }
  if(crc8_ow(data, sizeof(data))) {
    //Util::log("eeprom crc load error\r\n");
    return false;
  }

  memcpy(&channels, &data, sizeof(channels));
  memcpy(&settings, &data[sizeof(channels)], sizeof(settings));

  return true;
}

uint8_t *TempControl::GetModbusStatusMem(uint16_t address, uint16_t size) {
  if (address + size > sizeof(chStatus))
    return nullptr;

  return ((uint8_t *)&chStatus + address);
}

uint8_t *TempControl::GetModbusChannelMem(uint16_t address, uint16_t size) {
  if (address + size > sizeof(channels))
    return nullptr;

  return ((uint8_t *)&channels + address);
}

uint16_t TempControl::GetSettings() {
  return settings;
}

void TempControl::SetSettings(uint16_t s) {
  settings = s & 0x0f;
}

void printHex(BaseSequentialStream *chp, uint8_t *data, int len) {
  for (int i = 0; i < len; i++)
    chprintf(chp, " %02x", data[i]);
}

void TempControl::Print(BaseSequentialStream *chp) {
  if (!Util::sConfig.GetTempControlEnable()) {
    chprintf(chp, "Temp control module in DISABLED state\r\n");
    return;
  }

  for (uint8_t i = 0; i < MaxChannels; i++) {
    chprintf(chp, "Channel %d (%s):\r\n", i + 1, (GetChEnabled(i)) ? "enabled" : "disabled");

    chprintf(chp, "  Sensor1:", i);
    if (memcmp(channels[i].id[0], idzero, 8)) {
      printHex(chp, channels[i].id[0], 8);
      if (OWire::owDriver.getOwList()->isSensorPresent(channels[i].id[0])) {
        chprintf(chp, " (t: %d) ", OWire::owDriver.getOwList()->GetTemperature(channels[i].id[0]) - 100 * 100);
      } else {
        chprintf(chp, " (offline!)");
      }
    } else {
      chprintf(chp, " n/a");
    }

    chprintf(chp, "\r\n  Sensor2:", i);
    if (memcmp(channels[i].id[1], idzero, 8)) {
      printHex(chp, channels[i].id[1], 8);
      if (OWire::owDriver.getOwList()->isSensorPresent(channels[i].id[1])) {
        chprintf(chp, " (t: %d) ", OWire::owDriver.getOwList()->GetTemperature(channels[i].id[1]) - 100 * 100);
      } else {
        chprintf(chp, " (offline!)");
      }
    } else {
      chprintf(chp, " n/a");
    }

    chprintf(chp, "\r\n  Temperature1: ", i);
    if (channels[i].temp[0] != 0xffff)
      chprintf(chp, "%d", channels[i].temp[0] - 100 * 100);
    else
      chprintf(chp, "n/a");

    chprintf(chp, "\r\n  Temperature2: ", i);
    if (channels[i].temp[1] != 0xffff)
      chprintf(chp, "%d", channels[i].temp[1] - 100 * 100);
    else
      chprintf(chp, "n/a");

    chprintf(chp, "\r\n\r\n");
  }
}

#define strBoolState(ch, n)((chStatus[(ch)].state & (1 << (n))) ? "on" : "off")

void TempControl::PrintStatus(BaseSequentialStream *chp) {
  if (!Util::sConfig.GetTempControlEnable()) {
    chprintf(chp, "Temp control module in DISABLED state\r\n");
    return;
  }
  chprintf(chp, "Temp control module status:\r\n");

  for (uint8_t i = 0; i < MaxChannels; i++) {
    chprintf(chp, "Channel %d %s\r\n", i + 1, (GetChEnabled(i)) ? "(enabled):" : "(disabled)");

    if (!GetChEnabled(i))
      continue;

    if (chStatus[i].temp[0] != 0xffff)
      chprintf(chp, "  temp1: %d\r\n", chStatus[i].temp[0] - 100 * 100);
    else
      chprintf(chp, "  temp1: n/a\r\n");

    if (chStatus[i].temp[1] != 0xffff)
      chprintf(chp, "  temp2: %d\r\n", chStatus[i].temp[1] - 100 * 100);
    else
      chprintf(chp, "  temp2: n/a\r\n");

    chprintf(chp, "  status: 0x%04x (0b", chStatus[i].state);
    Util::PrintBin(chStatus[i].state, 16, 4);
    chprintf(chp, ")\r\n");

    chprintf(chp, "    temperature 1 ok:   %s\r\n", strBoolState(i, 0));
    chprintf(chp, "    temperature 2 ok:   %s\r\n", strBoolState(i, 1));
    chprintf(chp, "    setup temp 1 ok:    %s\r\n", strBoolState(i, 2));
    chprintf(chp, "    setup temp 2 ok:    %s\r\n", strBoolState(i, 3));
    chprintf(chp, "    safety check error: %s\r\n", strBoolState(i, 4));
    chprintf(chp, "    long loop needs heating:  %s\r\n", strBoolState(i, 5));
    chprintf(chp, "    short loop needs heating: %s\r\n", strBoolState(i, 6));
    chprintf(chp, "    long loop overheated:  %s\r\n", strBoolState(i, 7));
    chprintf(chp, "    short loop overheated: %s\r\n", strBoolState(i, 8));
    chprintf(chp, "    channel needs: %s\r\n", strBoolState(i, 9));
    chprintf(chp, "    channel ON:    %s\r\n", strBoolState(i, 10));

    chprintf(chp, "\r\n");
  }

  uint16_t regBuffer16 = Digital::output.GetBinaryVal();
  chprintf(chp, "control register : \r\n  0x%04x\r\n  0b", regBuffer16);
  Util::PrintBin(regBuffer16, 16, 4);
  chprintf(chp, "\r\n\r\n");
}

msg_t TempControl::SendMessage(TempControlCommand msg) {
  return chMsgSend(thread_ref, reinterpret_cast<msg_t>(&msg));
}

} // extern C
