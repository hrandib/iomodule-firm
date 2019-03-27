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

#include "owmaster_impl.h"
#include "digitalin.h"
#include "digitalout.h"
#include "analogin.h"
#include "order_conv.h"
#include "chprintf.h"
#include "sconfig.h"
#include "onewire.h"
#include "crc8.h"
#include "chmsg.h"

#if BOARD_VER == 1
#include "analogout.h"
#endif

#include <array>

using Utils::htons;
using Utils::ntohs;
using Utils::htonl;

OWMaster owMaster;

extern "C" {

static systime_t lastNetScan = 0;
static systime_t lastNetQueryTemp = 0;

void OWMaster::ExecNetScan() {
  lastNetScan = 0;
}

void OWMaster::ExecMeasurementCycle() {
  lastNetQueryTemp = 0;
}

bool OWMaster::Process18B20GetTemp(int listPosition) {
  bool res;
  uint8_t *id = OWire::owDriver.getOwList()->GetOWIDByListPosition(listPosition);
  if (!id)
    return false;

  res = OWire::owDriver.Select(id);
  if (!res)
    return res;

  res = OWire::owDriver.SendCommand((uint8_t)OWire::Command::ReadScratcpad);
  if (!res)
    return res;

  uint8_t sc[9];
  res = OWire::owDriver.Read(sc, sizeof(sc));
  if (!res)
    return res;

  if (!crc8(sc, 9)) {
    chprintf((BaseSequentialStream*)&SD1, "read crc error\r\n");
    return res;
  } else {
    chprintf((BaseSequentialStream*)&SD1, "read crc ok\r\n");
  }

  // if resolution not 12 bit
  if ((sc[4] & 0x60) != 0x60) {
    chprintf((BaseSequentialStream*)&SD1, "resolution warning: %02x\r\n", (sc[4] & 0x60) >> 5);
  }

  chprintf((BaseSequentialStream*)&SD1, "data: %02x %02x %02x %02x %02x %02x %02x %02x %02x\r\n", sc[0], sc[1], sc[2], sc[3], sc[4], sc[5], sc[6], sc[7], sc[8]);

  // target format: (value + 100) * 100
  uint16_t temp = sc[0] + (uint16_t)(sc[1] << 8);

  uint16_t t2 = (temp & 0x07ff) >> 4;
  // 2 ^ -1  ...  2 ^ -4
  t2 = t2 * 100 + (temp & 8 ? 50 : 0)  + (temp & 4 ? 25 : 0)  + (temp & 2 ? 12 : 0) + (temp & 1 ? 6 : 0);
  t2 = (100 * 100 + ((temp & 0x8000) ? -t2 : t2));

  res = OWire::owDriver.getOwList()->SetTemperature(id, t2);
  if (!res)
    return res;

  return true;
}

void OWMaster::Process()
{
  if (!OWire::owDriver.Ready())
    return;

  // scan network. 60 min between scans
  if (!mesStarted && (lastNetScan == 0 || chVTGetSystemTimeX() - lastNetScan > 60 * 60 * 1000)) {
    chprintf((BaseSequentialStream*)&SD1, "OW network scan\r\n");
    OWire::owDriver.Search(); // search all devices
    lastNetScan = chVTGetSystemTimeX();
    return;
  }

  // start convert. 60 sec between temperature measuring
  if (!mesStarted && (lastNetQueryTemp == 0 || chVTGetSystemTimeX() - lastNetQueryTemp > 60 * 1000)) {
    lastNetQueryTemp = chVTGetSystemTimeX();

    if (!OWire::owDriver.getOwList()->Count()) {
      return;
    }

    bool res = OWire::owDriver.SendCommandAllNet((uint8_t)OWire::Command::ConvertT); // start convert T all sensors
    mesStarted = true;
    chprintf((BaseSequentialStream*)&SD1, "OW start convert [%d] %s\r\n", OWire::owDriver.getOwList()->Count(), res ? "ok" : "err");
    return;
  }

  // read temp after 1 sec timeout
  if (mesStarted && (lastNetQueryTemp == 0 || chVTGetSystemTimeX() - lastNetQueryTemp > 1 * 1000)) { // 1s after start convert
    int listlen = OWire::owDriver.getOwList()->Count();
    int scanIndx = 0;
    do {
      if (!Process18B20GetTemp(scanIndx))
        break;

      scanIndx++;
    } while (scanIndx < listlen);

    lastNetQueryTemp = chVTGetSystemTimeX();
    mesStarted = false;
    return;
  }

  return;
}

void OWMaster::Init() {
  mesStarted = false;
  OWire::owDriver.Init(GPIOA, 12, GPIOB, 5, this);
  start(NORMALPRIO + 12);
}

void OWMaster::main() {
  setName("OWMaster");
  sleep(MS2ST(300));

  while(true) {
    if (Util::sConfig.GetOWEnable())
      Process();

    /* Searching for a queued message then retrieving it.*/
    chSysLock();
    bool haveMsg = chMsgIsPendingI(currp);
    chSysUnlock();
    if (haveMsg) {
      thread_t *tp = chMsgWait();
       OWMasterCommand *cmd = reinterpret_cast<OWMasterCommand*>(chMsgGet(tp));

       switch (*cmd) {
       default:
         break;
       case owcmdRescanNetwork:
         ExecNetScan();
         break;
       case owcmdMeasurement:
         ExecMeasurementCycle();
         break;
       case owcmdPrintOWList:
         OWire::OWList *owlist = OWire::owDriver.getOwList();
         if (owlist)
           owlist->Print((BaseSequentialStream*)&SD1, false);
         else
           chprintf((BaseSequentialStream*)&SD1, "OW list not found\r\n");
         break;
       }

      chMsgRelease(tp, MSG_OK);
    }

    sleep(MS2ST(200));
  }
}

msg_t OWMaster::SendMessage(OWMasterCommand msg) {
  return chMsgSend(thread_ref, reinterpret_cast<msg_t>(&msg));
}

} // extern C
