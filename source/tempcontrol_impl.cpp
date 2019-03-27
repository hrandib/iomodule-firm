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

void TempControl::Process()
{
  if (!OWire::owDriver.Ready())
    return;


  return;
}

void TempControl::Init() {
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

void TempControl::Print(BaseSequentialStream *chp) {
  if (!Util::sConfig.GetTempControlEnable()) {
    chprintf(chp, "Temp control module in DISABLED state");
    return;
  }

}

msg_t TempControl::SendMessage(TempControlCommand msg) {
  return chMsgSend(thread_ref, reinterpret_cast<msg_t>(&msg));
}

} // extern C
