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
#ifndef MODBUS_IMPL_H
#define MODBUS_IMPL_H

#include "mb.h"
#include "ch_extended.h"

static const uint8_t *UniqProcessorId = (uint8_t *) 0x1FFFF7E8;
static const uint8_t UniqProcessorIdLen = 12;

class Modbus : Rtos::BaseStaticThread<512>
{
private:
  enum {
    DEV_ID = 15
  };

  bool InitModbus()
  {
    eMBErrorCode eStatus;

    eStatus = eMBInit(MB_RTU, DEV_ID, 1, 115200, MB_PAR_NONE);
    if (eStatus != MB_ENOERR) {
      return FALSE;
    }

    eStatus = eMBSetSlaveID(DEV_ID, TRUE, UniqProcessorId, UniqProcessorIdLen);
    if (eStatus != MB_ENOERR) {
      return FALSE;
    }

    eStatus = eMBEnable();
    if (eStatus != MB_ENOERR) {
      return FALSE;
    }

    pxMBPortCBTimerExpired();

    return TRUE;
  }

public:
  void Init()
  {
    start(NORMALPRIO);
  }
  void main() override
  {
    setName("Modbus");
    while(InitModbus() != true) {
      sleep(MS2ST(300));
    }
    sleep(MS2ST(10));
    while(true) {
      eMBPoll();
    }
  }
};


#endif // MODBUS_IMPL_H
