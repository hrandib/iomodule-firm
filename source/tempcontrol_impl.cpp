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

void TempControl::Process()
{
  if (!OWire::owDriver.Ready())
    return;


  return;
}

void TempControl::Init() {
  for (int i = 0; i < MaxChannels; i++) {
    memset(channels[i].id[0], 0, 8);
    memset(channels[i].id[1], 0, 8);
    channels[i].temp[0] = 0xffff;
    channels[i].temp[1] = 0xffff;
  }

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
       }

      chMsgRelease(tp, MSG_OK);
    }

    sleep(MS2ST(300));
  }
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

void printHex(BaseSequentialStream *chp, uint8_t *data, int len) {
  for (int i = 0; i < len; i++)
    chprintf(chp, " %02x", data[i]);
}

void TempControl::Print(BaseSequentialStream *chp) {
  if (!Util::sConfig.GetTempControlEnable()) {
    chprintf(chp, "Temp control module in DISABLED state\r\n");
    return;
  }

  for (int i = 0; i < MaxChannels; i++) {
    chprintf(chp, "Channel %d:\r\n", i + 1);

    chprintf(chp, "  Sensor1:", i);
    if (memcmp(channels[i].id[0], idzero, 8)) {
      printHex(chp, channels[i].id[0], 8);
      if (!OWire::owDriver.getOwList()->isSensorPresent(channels[i].id[0])) {
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
      if (!OWire::owDriver.getOwList()->isSensorPresent(channels[i].id[1])) {
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

msg_t TempControl::SendMessage(TempControlCommand msg) {
  return chMsgSend(thread_ref, reinterpret_cast<msg_t>(&msg));
}

} // extern C
