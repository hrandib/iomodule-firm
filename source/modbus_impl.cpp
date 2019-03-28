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

#include "modbus_impl.h"
#include <string.h>
#include "sconfig.h"
#include "digitalin.h"
#include "digitalout.h"
#include "analogin.h"
#include "order_conv.h"
#include "onewire.h"

#if BOARD_VER == 1
#include "analogout.h"
#endif

#include <array>

static const uint8_t *UniqProcessorId = (uint8_t *) 0x1FFFF7E8;
static const uint8_t UniqProcessorIdLen = 12;

using Utils::htons;
using Utils::ntohs;
using Utils::htonl;

Modbus modbus;

enum Range {
  R_AnalogInputStart = 32,
  R_AnalogInputSize = 10,
  R_CounterStart = 64,
  R_CounterSize = 14 * 2,
  R_DigitalInputStart = 96,
  R_DigitalInputSize = 1,
  R_AnalogOutputStart = 128,
  R_AnalogOutputSize = 4,
  R_DigitalOutputStart = 160,
  R_DigitalOutputSize = 4,
  R_SystemStatStart = 192,
  R_SystemStatSize = 2,
  R_SystemSetupStart = 250,
  R_SystemSetupSize = 2,
  R_TempCntrlStart = 300,
  R_TempCntrlSize = 100,  // TBC
  // ow records. record = 12b or 6 reg
  R_OWStart = 400,
  R_OWSize = 96    // 16 records 96 reg * 2b = 192b
};

extern "C" {

bool MBAddressInDiap(USHORT address, USHORT nregs, USHORT mbDiapAddress, USHORT mbDiapSize) {
  return ( (address >= mbDiapAddress) &&
           (address < mbDiapAddress + mbDiapSize) &&
           (address + nregs <= mbDiapAddress + mbDiapSize)
         );
}

  /**
   * Modbus slave input register callback function.
   *
   * @param pucRegBuffer input register buffer
   * @param usAddress input register address
   * @param usNRegs input register number
   *
   * @return result
   */
  eMBErrorCode eMBRegInputCB(UCHAR* pucRegBuffer, USHORT usAddress, USHORT usNRegs)
  {
    eMBErrorCode eStatus = MB_ENOERR;
    int iRegIndex;
    uint16_t* regBuffer16 = (uint16_t*)pucRegBuffer;
    /* it already plus one in modbus function method. */
    --usAddress;
    //Uptime
    if (MBAddressInDiap(usAddress, usNRegs, R_SystemStatStart, R_SystemStatSize)) {
      uint32_t be32 = htonl(uptimeCounter.load());

      uint16_t buf[2] = {0};
      buf[0] = uint16_t(be32 & 0xFFFF);
      buf[1] = uint16_t(be32 >> 16);
      memcpy(regBuffer16, &buf[usAddress - R_SystemStatStart], usNRegs * 2);
    }
    //Digital inputs data
    else if(usAddress == R_DigitalInputStart) {
      if(usNRegs == R_DigitalInputSize) {
        *regBuffer16 = htons(Digital::input.GetBinaryVal());
      }
      else {
        eStatus = MB_ENOREG;
      }
    }
    //Counters data, the number of registers must be even
    else if(usAddress >= R_CounterStart && usAddress < R_CounterStart + R_CounterSize) {
      iRegIndex = (int)(usAddress - R_CounterStart);
      if((usNRegs + iRegIndex) <= R_CounterSize && (usNRegs & 0x01) == 0) {
        auto counters = Digital::input.GetCounters();
        while(usNRegs > 0) {
          uint32_t be32 = htonl(counters[(size_t)iRegIndex]);
          *regBuffer16++ = uint16_t(be32 & 0xFFFF);
          *regBuffer16++ = uint16_t(be32 >> 16);
          ++iRegIndex;
          usNRegs -= 2;
        }
      }
      else {
        eStatus = MB_ENOREG;
      }
    }
    //Analog inputs data
    else if(usAddress >= R_AnalogInputStart && usAddress < R_AnalogInputStart + R_AnalogInputSize) {
      iRegIndex = (int)(usAddress - R_AnalogInputStart);
      if((usNRegs + iRegIndex) <= R_AnalogInputSize) {
        auto inputs = Analog::input.GetSamples();
        while(usNRegs > 0) {
          *regBuffer16++ = htons(inputs[(size_t)iRegIndex]);
          ++iRegIndex;
          --usNRegs;
        }
      }
      else {
        eStatus = MB_ENOREG;
      }
    }
    //1-Wire sensors data
    else if((usAddress >= R_OWStart) && (usAddress < R_OWStart + R_OWSize) &&
            (usAddress + usNRegs <= R_OWStart + R_OWSize)) {
      chprintf((BaseSequentialStream*)&SD1, "mb get %d %d\r\n", usAddress, usNRegs);
      iRegIndex = 0;
      uint8_t *data = OWire::owDriver.getOwList()->GetModbusMem((usAddress - R_OWStart) * 2, usNRegs * 2);
      if (data)
        memcpy(pucRegBuffer, data, usNRegs * 2);
      else
        eStatus = MB_ENOREG;
    }
    else {
      eStatus = MB_ENOREG;
    }
    return eStatus;
  }

  /**
   * Modbus slave holding register callback function.
   *
   * @param pucRegBuffer holding register buffer
   * @param usAddress holding register address
   * @param usNRegs holding register number
   * @param eMode read or write
   *
   * @return result
   */
  eMBErrorCode eMBRegHoldingCB(UCHAR * pucRegBuffer, USHORT usAddress,
                               USHORT usNRegs, eMBRegisterMode eMode)
  {
    enum RegType {
      regReadWrite,
      regSet,
      regClear,
      regToggle
    };
    eMBErrorCode eStatus = MB_ENOERR;
    int iRegIndex;
    uint16_t* regBuffer16 = (uint16_t*)pucRegBuffer;
    /* it already plus one in modbus function method. */
    --usAddress;
    //Digital output data, write only part (set/clear/toggle)
    if(eMode == MB_REG_READ &&
       (usAddress > R_DigitalOutputStart || (usAddress == R_DigitalOutputStart && usNRegs > 1))) {
      return MB_ENOREG;
    }
    if(usAddress >= R_DigitalOutputStart) {
      iRegIndex = (int)(usAddress - R_DigitalOutputStart);
      if((usNRegs + iRegIndex) <= R_DigitalOutputSize) {
        while(usNRegs > 0) {
          Digital::OutputCommand cmd{};
          switch(RegType(iRegIndex))
          {
          case regReadWrite: {
              if(eMode == MB_REG_READ) {
                Digital::output.SendMessage(cmd);
                *regBuffer16++ = htons(uint16_t(cmd.GetValue()));
              }
              else {
                cmd.Set(decltype(cmd)::Mode::Write, ntohs(*regBuffer16++));
                Digital::output.SendMessage(cmd);
              }
            }
            break;
          case regSet:
            //only set
            if(usNRegs == 1) {
              cmd.Set(decltype(cmd)::Mode::Set, *regBuffer16++);
              Digital::output.SendMessage(cmd);
            }
            //set and clear
            else {
              cmd.Set(decltype(cmd)::Mode::SetAndClear, ntohs(regBuffer16[0]) | ((uint32_t)ntohs(regBuffer16[1]) << 16));
              Digital::output.SendMessage(cmd);
              regBuffer16 += 2;
              --usNRegs;
              ++iRegIndex;
            }
            break;
          case regClear:
            cmd.Set(decltype(cmd)::Mode::Clear, *regBuffer16++);
            Digital::output.SendMessage(cmd);
            break;
          case regToggle:
            cmd.Set(decltype(cmd)::Mode::Toggle, *regBuffer16);
            Digital::output.SendMessage(cmd);
            break;
          }
          --usNRegs;
          ++iRegIndex;
        }
      }
      else {
        eStatus = MB_ENOREG;
      }
    }
#if BOARD_VER == 1
    else if(usAddress >= R_AnalogOutputStart) {
      iRegIndex = (int)(usAddress - R_AnalogOutputStart);
      if((usNRegs + iRegIndex) <= R_AnalogOutputSize) {
        Analog::OutputCommand cmd{};
        if(eMode == MB_REG_READ) {
          Analog::output.SendMessage(cmd);
          while(usNRegs > 0) {
            *regBuffer16++ = htons(cmd.GetValue((size_t)iRegIndex));
            ++iRegIndex;
            --usNRegs;
          }
        }
        else {
          while(usNRegs > 0) {
            auto val = ntohs(*regBuffer16++);
            if(val > Analog::Output::Resolution) {
              return MB_EINVAL;
            }
            cmd.SetValue((size_t)iRegIndex, val);
            ++iRegIndex;
            --usNRegs;
          }
          Analog::output.SendMessage(cmd);
        }
      }
      else {
        eStatus = MB_ENOREG;
      }
    }
#endif
    else {
      eStatus = MB_ENOREG;
    }
    return eStatus;
  }
}

bool Modbus::InitModbus()
{
  eMBErrorCode eStatus;
  uint8_t devID = 5;//(uint8_t)devID_.load();
  eStatus = eMBInit(MB_RTU, devID, 1, 115200, MB_PAR_NONE);
  if (eStatus != MB_ENOERR) {
    return FALSE;
  }

  eStatus = eMBSetSlaveID(devID, TRUE, UniqProcessorId, UniqProcessorIdLen);
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

void Modbus::Init()
{
  uint32_t nvID = Util::sConfig.GetModbusAddress();
  if(!nvID || nvID >= MB_ADDRESS_MAX) {
    devID_ = FALLBACK_ID;
  }
  else {
    devID_ = nvID;
  }
  start(NORMALPRIO + 13);
}

void Modbus::main()
{
  setName("Modbus");
  while(InitModbus() != true) {
    sleep(MS2ST(300));
  }
  while(true) {
    eMBPoll();
  }
}

eMBErrorCode Modbus::SetID(uint8_t id)
{
  if(!id || id >= MB_ADDRESS_MAX) {
    return MB_EINVAL;
  }
  eMBSetSlaveID(id, TRUE, UniqProcessorId, UniqProcessorIdLen);
  devID_ = id;
  return MB_ENOERR;
}

uint8_t Modbus::GetID()
{
  return (uint8_t)devID_;
}
