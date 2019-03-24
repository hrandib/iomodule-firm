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
#include "onewire.h"
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
static uint8_t lastScanIndx = 0;

void OWMaster::ExecNetScan() {
  lastNetScan = 0;
}

void OWMaster::Process()
{
//  OWire::DS18B20Driver.Process();

  if (!OWire::owDriver.Ready())
    return;

  // scan network. 60 min between scans
  if (lastNetScan == 0 || chVTGetSystemTimeX() - lastNetScan > 60 * 60 * 1000) {
    chprintf((BaseSequentialStream*)&SD1, "OW network scan\r\n");
    OWire::owDriver.Search(); // search all devices
    lastNetScan = chVTGetSystemTimeX();
    return;
  }
/*
  // start convert. 60 sec between temperature measuring
  if (lastNetQueryTemp == 0 || chVTGetSystemTimeX() - lastNetQueryTemp > 60 * 1000) {
    OWire::DS18B20Driver.StartConvert(); // start convert over all network
    lastScanIndx = 0;
    lastNetQueryTemp = chVTGetSystemTimeX();
    chprintf((BaseSequentialStream*)&SD1, "OW start convert\r\n");
    return;
  }

  if (OWire::DS18B20Driver.ReadScratchPad(id[lastScanIndx], &sp)) {
    temp[lastScanIndx] = sp.temp;

    if (sp.resolition != OWire::DS18B20Res12bit) {
      OWire::DS18B20Driver.SetResolution(id[lastScanIndx], OWire::DS18B20Res12bit);
    }

    lastScanIndx++;
    if (lastScanIndx > maxScanIndx)
      lastScanIndx = 0;
  }
*/
  return;
}

void OWMaster::Init() {
  OWire::owDriver.Init(GPIOA, 12, GPIOB, 5, this);
//  OWire::DS18B20Driver.Init(OWDriver);
  start(NORMALPRIO + 12);
}

void OWMaster::main() {
  setName("OWMaster");
  sleep(MS2ST(300));

  while(true) {
    Process();

    /* Searching for a queued message then retrieving it.*/
    chSysLock();
    bool haveMsg = chMsgIsPendingI(currp);
    chSysUnlock();
    if (haveMsg) {
      thread_t *tp = chMsgWait();
       OWMasterCommand *cmd = reinterpret_cast<OWMasterCommand*>(chMsgGet(tp));

       switch (*cmd) {
       case owcmdRescanNetwork:
         ExecNetScan();
         break;
       case owcmdPrintOWList:
         OWire::OWList *owlist = OWire::owDriver.getOwList();
         if (owlist)
           owlist->Print((BaseSequentialStream*)&SD1, true);
         else
           chprintf((BaseSequentialStream*)&SD1, "OW list not found\r\n");
         break;
       }

      chMsgRelease(tp, MSG_OK);
    }

    sleep(MS2ST(1));
  }
}

msg_t OWMaster::SendMessage(OWMasterCommand msg) {
  return chMsgSend(thread_ref, reinterpret_cast<msg_t>(&msg));
}

} // extern C
