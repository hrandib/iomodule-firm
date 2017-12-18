/*
 * Copyright (c) 2017 Dmytro Shestakov
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

// http://www.leoniv.diod.club/articles/wake/wake.html

#ifndef WAKE_BASE_H
#define WAKE_BASE_H

#include "ch_extended.h"
#include "hal.h"
#include "crc8.h"

namespace Wk {

  static constexpr size_t INSTRUCTION_SET_VER_MAJOR = 2;
  static constexpr size_t INSTRUCTION_SET_VER_MINOR = 1;

  enum {
    CRC_INIT = 0xDE,
    FEND = 0xC0,    //Frame END
    FESC = 0xDB,    //Frame ESCape
    TFEND = 0xDC,    //Transposed Frame END
    TFESC = 0xDD,    //Transposed Frame ESCape
    DEFAULT_ADDRESS = 15
  };

  enum State {
    WAIT_FEND = 0,     //ожидание приема FEND
    ADDR,     //ожидание приема адреса            //передача адреса
    CMD,      //ожидание приема команды            //передача команды
    NBT,      //ожидание приема количества байт в пакете //передача количества байт в пакете
    DATA     //прием данных               //передача данных
  };

  enum Cmd {
    C_NOP,    //нет операции
    C_ERR,    //ошибка приема пакета
    C_ECHO,    //передать эхо
    C_GETINFO,
    C_SETNODEADDRESS,
    C_GETGROUPADDRESS,
    C_SETGROUPADDRESS = C_GETGROUPADDRESS,
    C_GETOPTIME,
    C_OFF,
    C_ON,
    C_ToggleOnOff,
    C_SAVESETTINGS,

    C_BASE_NUMBER
  };

  enum Err {
    ERR_NO, //no error
    ERR_TX, //Rx/Tx error
    ERR_BU, //device busy error
    ERR_RE, //device not ready error
    ERR_PA, //parameters value error
    ERR_NI, //Command not impl
    ERR_NR, //no replay
    ERR_NC, //no carrier
    ERR_ADDRFMT,  //new address is wrong
    ERR_EEPROMUNLOCK //EEPROM wasn't unlocked
  };

  enum DeviceType {
    DevNull,
    DevLedDriver = 0x01,
    DevSwitch = 0x02,
    DevRgbDriver = 0x04,
    DevGenericIO = 0x08,
    DevSensor = 0x10,
    DevPowerSupply = 0x20,
    //    Reserved = 0x40,
    DevCustom = 0x80
  };

  enum AddrType {
    addrGroup,
    addrNode
  };

  class WakeBase
  {
  public:
    static constexpr size_t WAKEDATABUFSIZE = 128;
    struct Packet {
      uint8_t addr;
      uint8_t cmd;
      uint8_t payloadSize;
      uint8_t buf[WAKEDATABUFSIZE];
      void NotImplemented()
      {
        payloadSize = 1;
        buf[0] = ERR_NI;
      }
      void ParameterError()
      {
        payloadSize = 1;
        buf[0] = ERR_PA;
      }
    };
  protected:
    static void RXerr(UARTDriver* uartp, uartflags_t e);
    static void RXend(UARTDriver* uartp);
    static void RXchar(UARTDriver* uartp, uint16_t c);
    static void TXend1(UARTDriver* uartp);
    static void TXend2(UARTDriver* uartp);

    static void SetDE(UARTDriver* uartp)
    {
      uartp->usart->CR3 &= ~USART_CR3_DMAR;
      WakeBase& wake = *reinterpret_cast<WakeBase*>(uartp->customData);
      if(wake.portDE_) {
        palSetPad(wake.portDE_, wake.pinDE_);
      }
    }
    static void ClearDE(UARTDriver* uartp)
    {
      uartp->usart->SR = ~USART_SR_RXNE;
      WakeBase& wake = *reinterpret_cast<WakeBase*>(uartp->customData);
      if(wake.portDE_) {
        palClearPad(wake.portDE_, wake.pinDE_);
      }
      uartp->usart->CR3 |= USART_CR3_DMAR;
    }

    uint8_t txBuf_[sizeof(Packet) + 8];
    Rtos::ThreadStayPoint stayPoint_;

    ioportid_t portDE_;
    uint16_t pinDE_;

    Packet packetData_;
    uint8_t ptr_;       //data index in Rx buffer
    State state_;
    uint8_t prevByte_;
    Crc::Crc8 crc_;

    size_t FillTxBuf();
  };

  //Every module should implement the same interface
  struct NullModule {
    static constexpr uint8_t GetDeviceMask()
    {
      return DevNull;
    }
    static void Init() { }
    static bool Process(WakeBase::Packet&)
    {
      return false;
    }
    static void SaveState() { }
    static void On() { }
    static void Off() { }
    static constexpr uint8_t GetDeviceFeatures()
    {
      return 0;
    }
    static void ToggleOnOff() { }
  };

  namespace _impl {

    template<typename...>
    struct DeviceFeaturesHelper;

    template<typename First, typename... Rest>
    struct DeviceFeaturesHelper<First, Rest...> {
      static constexpr uint8_t Get(uint8_t deviceMask)
      {
        return First::GetDeviceMask() == deviceMask ? First::GetDeviceFeatures() :
                                                      DeviceFeaturesHelper<Rest...>::Get(deviceMask);
      }
    };

    template<>
    struct DeviceFeaturesHelper<> {
      static constexpr uint8_t Get(uint8_t)
      {
        return 0;
      }
    };

    template<typename... Modules>
    struct ModuleList {
      static constexpr uint8_t GetDeviceMask()
      {
        return (NullModule::GetDeviceMask() | ... | Modules::GetDeviceMask());
      }
      static void Init()
      {
        (NullModule::Init(), ..., Modules::Init());
      }
      static bool Process(WakeBase::Packet& packet)
      {
        return (NullModule::Process(packet) || ... || Modules::Process(packet));
      }
      static constexpr uint8_t GetDeviceFeatures(uint8_t deviceMask)
      {
        return DeviceFeaturesHelper<Modules...>::Get(deviceMask);
      }
      static void On()
      {
        (NullModule::On(), ..., Modules::On());
      }
      static void Off()
      {
        (NullModule::Off(), ..., Modules::Off());
      }
      static void ToggleOnOff()
      {
        (NullModule::ToggleOnOff(), ..., Modules::ToggleOnOff());
      }
    };
  }

  using _impl::ModuleList;

  template<typename... Modules>
  class Wake : public WakeBase, public Rtos::BaseStaticThread<256>
  {
  private:
    using Base = WakeBase;
    using ModuleList_ = ModuleList<Modules...>;

    UARTDriver* uartd_;
    UARTConfig conf_;

    bool IsNodeAddress()
    {
      uint8_t taddr = packetData_.buf[0];
      return taddr == (~packetData_.buf[1] & 0xFF)
             && ((taddr && taddr < 80) || (taddr > 112 && taddr < 128));
    }
    bool IsGroupAddress()
    {
      uint8_t taddr = packetData_.buf[0];
      return taddr == (~packetData_.buf[1] & 0xFF)
             && taddr > 79 && taddr < 96;
    }
    bool IsNotBroadcast()
    {
      return packetData_.addr != 0;
    }

    void ProcessDefault(Cmd cmd)
    {
      if(cmd) {
        switch(cmd) {
        case C_NOP:
        case C_ECHO:
        case C_BASE_NUMBER: // warning suppress
          break;
        case C_ERR:
          cmd = C_NOP;
          return;
        case C_GETINFO:
          //Common device info
          if(!packetData_.payloadSize) {
            packetData_.buf[0] = ERR_NO;
            packetData_.buf[1] = ModuleList_::GetDeviceMask();
            packetData_.buf[2] = INSTRUCTION_SET_VER_MAJOR << 4 | INSTRUCTION_SET_VER_MINOR;
            packetData_.payloadSize = 3;
          }
          //Info about single logical device
          else if(packetData_.payloadSize == 1) {
            if(packetData_.buf[0] < 7) {
              const uint8_t deviceMask = uint8_t(1 << packetData_.buf[0]);
              //Device is available
              if(ModuleList_::GetDeviceMask() & deviceMask) {
                packetData_.buf[0] = ERR_NO;
                packetData_.buf[1] = ModuleList_::GetDeviceFeatures(deviceMask);
                packetData_.payloadSize = 2;
              }
              //device not available
              else {
                packetData_.NotImplemented();
              }
            }
            //else if(packetData.buf[0] == 7) //custom device
          }
          else {
            packetData_.ParameterError();
          }
          break;
        case C_SETNODEADDRESS:
          packetData_.NotImplemented();
          break;
        case C_SETGROUPADDRESS:
          packetData_.NotImplemented();
          break;
        case C_GETOPTIME:
          packetData_.NotImplemented();
          break;
        case C_OFF:
          if(!packetData_.payloadSize) {
            packetData_.buf[0] = Wk::ERR_NO;
            ModuleList_::Off();
            packetData_.payloadSize = 1;
          }
          else {
            packetData_.ParameterError();
          }
          break;
        case C_ON:
          if(!packetData_.payloadSize) {
            packetData_.buf[0] = Wk::ERR_NO;
            ModuleList_::On();
            packetData_.payloadSize = 1;
          }
          else {
            packetData_.ParameterError();
          }
          break;
        case C_ToggleOnOff:
          if(!packetData_.payloadSize) {
            packetData_.buf[0] = Wk::ERR_NO;
            ModuleList_::ToggleOnOff();
            packetData_.payloadSize = 1;
          }
          else {
            packetData_.ParameterError();
          }
          break;
        case C_SAVESETTINGS:
          packetData_.NotImplemented();
          break;
        default:
          if(!ModuleList_::Process(packetData_)) {
            packetData_.NotImplemented();
          }
        } //Switch
      }
    }
  public:
    Wake(UARTDriver& uartd, size_t baud, ioportid_t portDE = 0, uint16_t pinDE = 0) : uartd_{&uartd},
      conf_{ TXend1, TXend2, RXend, RXchar, RXerr, baud, 0, 0, USART_CR3_HDSEL }
    {
      portDE_ = portDE;
      pinDE_ = pinDE;
      uartd.customData = static_cast<WakeBase*>(this);
    }
    void Init()
    {
      palSetPadMode(portDE_, pinDE_, PAL_MODE_OUTPUT_PUSHPULL);
      palClearPad(portDE_, pinDE_);
      ModuleList_::Init();
      uartStart(uartd_, &conf_);
      start(NORMALPRIO - 1);
    }

    void main() override
    {
      while(true) {
        msg_t msg = stayPoint_.Suspend();
        if(msg == MSG_OK || msg == MSG_RESET || msg == MSG_TIMEOUT) {
          continue;
        }
        ProcessDefault((Cmd)msg);
        if(IsNotBroadcast()) {
          SetDE(uartd_);
          size_t size = FillTxBuf();
          uartStartSend(uartd_, size, txBuf_);
        }
      }
    }
  };
}//Wake

#endif // WAKE_BASE_H
