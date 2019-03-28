
#include "onewire.h"
#include "crc8.h"
//#include "stm32f10x.h"
#include "gpio.h"

#include "chprintf.h"
#include <string.h> //for memset

template<typename... Ts>
static inline void log(const char* str, Ts... ts) {
  chprintf((BaseSequentialStream*)&SD1, str, ts...);
}

enum OwOperation {
  owopDone,
  owopReset,
  owopReadBits,
  owopWriteBits,
  owopNA,
};

namespace OWire {
#define owSend1() (palClearPad(txPort, txPin))
#define owSend0() (palSetPad(txPort, txPin))
#define owRead() (palReadPad(rxPort, rxPin))

//using OWPinTx = Pa12;
//using OWPinRx = Pb5;
//using OWPinTest = Pb4;

  // Timer

  static enum OwOperation CurrentOperation = owopDone;
  static uint8_t CurrentOperationPhase = 0;
  static uint8_t *CurrentOperationValue = nullptr;
  static int CurrentOperationValueBitCnt = 0;

  // global declarations for use in class and timer callback
  static GPIO_TypeDef *txPort;
  static uint8_t txPin;
  static GPIO_TypeDef *rxPort;
  static uint8_t rxPin;

  void TimerDisable(GPTDriver* gpt) {
    Rtos::SysLockGuardFromISR lock{};
    gptStopTimerI(gpt);
  };

  void TimerOneShot(GPTDriver* gpt, gptcnt_t interval) {
    Rtos::SysLockGuardFromISR lock{};
    gptStopTimerI(gpt);
    gptStartOneShotI(gpt, interval);
  };

  void TimerHandler(GPTDriver* gpt) {
    //OWDriver& owd = *static_cast<OWDriver*>(gpt->customData);

    switch (CurrentOperation) {
      case owopDone:
        TimerDisable(gpt);
        break;

      case owopReset:
        switch (CurrentOperationPhase) {
          case 0:
            owSend1();
            *CurrentOperationValue = 0;
            CurrentOperationPhase++;
            TimerOneShot(gpt, 100);
            break;
          case 1:
            *CurrentOperationValue = owRead();
            CurrentOperationPhase++;
            TimerOneShot(gpt, 450); // min 480 mks from `owSend1()`
          break;
          case 2:
            CurrentOperation = owopDone;
          break;
        }
      break;

      case owopReadBits:
        switch (CurrentOperationPhase) {
          case 0:
            if (!CurrentOperationValueBitCnt) {
              CurrentOperation = owopDone;

              break;
            };

            owSend0();
            palTogglePad(GPIOB, 4);
            CurrentOperationPhase++;
            TimerOneShot(gpt, 1);
            return;
          case 1:
            owSend1();
            CurrentOperationPhase++;
            TimerOneShot(gpt, 2);
            return;
          case 2:
            CurrentOperationValueBitCnt--;

            // reading window - (1mks after `owSend1()`) .. (15mks from start(`owSend0()`))
            bool bit = owRead() & 0x01;
            palTogglePad(GPIOB, 4);
            *CurrentOperationValue = (*CurrentOperationValue >> 1) & 0xff;
            *CurrentOperationValue |= (bit ? 0x80 : 0x00);

            if (CurrentOperationValueBitCnt % 8 == 0) {
              CurrentOperationValue++;
              *CurrentOperationValue = 0;
            }
            CurrentOperationPhase = 0;
            TimerOneShot(gpt, 60); // min 60 mks from `owSend1()`
            return;
        }
      break;

      case owopWriteBits:
        switch (CurrentOperationPhase) {
          case 0:
            if (!CurrentOperationValueBitCnt) {
              CurrentOperation = owopDone;
              break;
            };

            owSend0();
            CurrentOperationPhase++;
            TimerOneShot(gpt, 10);
            return;
          case 1:
            CurrentOperationValueBitCnt--;
            if (*CurrentOperationValue & 0x01)
              owSend1();
            if (CurrentOperationValueBitCnt % 8 == 0) {
              CurrentOperationValue++;
            } else {
              *CurrentOperationValue = *CurrentOperationValue >> 1;
            }
            CurrentOperationPhase++;
            TimerOneShot(gpt, 40);
            return;
          case 2:
            owSend1();
            CurrentOperationPhase = 0;
            TimerOneShot(gpt, 20); // min 60 mks from `owSend1()`
            return;
        }
      break;

      default:
        CurrentOperation = owopDone;
        TimerDisable(gpt);
      break;
    }
    return;
  }
  const GPTConfig gptconf_{1000000, TimerHandler, 0, 0};

  // OWDriver

  bool OWDriver::Reset(bool *networkHaveDevice) {
    uint8_t b;
    CurrentOperationValue = &b;

    owSend1();
    parentThread->sleep(MS2ST(1));

    *networkHaveDevice = false;
    if (owRead() != PAL_HIGH) {
      return false;
    }

    owSend0();
    CurrentOperation = owopReset;
    CurrentOperationPhase = 0;
    gptStartOneShot(GPTD_, 500);

    parentThread->sleep(MS2ST(2)); //ms

    if (CurrentOperation != owopDone){
      owSend1();
      gptStopTimer(GPTD_);
      CurrentOperation = owopDone;
      return false;
    }

    *networkHaveDevice = (*CurrentOperationValue == 0);

    CurrentOperationValue = nullptr;
    return true;
  }

  bool OWDriver::_Read(uint8_t *bits, int bitCount) {
    if (owRead() != PAL_HIGH)
      return false;

    CurrentOperation = owopReadBits;
    CurrentOperationPhase = 0;
    CurrentOperationValue = bits;
    *CurrentOperationValue = 0;
    CurrentOperationValueBitCnt = bitCount;
    gptStartOneShot(GPTD_, 100);

    for (int i = 0; i < 10; i++) {
      parentThread->sleep(MS2ST(1)); //ms
      if (CurrentOperation == owopDone)
        break;
    }

    CurrentOperationValue = nullptr;
    gptStopTimer(GPTD_);

    if (CurrentOperation != owopDone){
      owSend1();
      chprintf((BaseSequentialStream*)&SD1, "rd op error: %d\r\n", CurrentOperation);
      gptStopTimer(GPTD_);
      CurrentOperation = owopDone;
      return false;
    }

    return true;
  }

  bool OWDriver::_Write(uint8_t *bits, int bitCount) {
    if (owRead() != PAL_HIGH)
      return false;

    CurrentOperation = owopWriteBits;
    CurrentOperationPhase = 0;
    CurrentOperationValue = bits;
    CurrentOperationValueBitCnt = bitCount;
    gptStartOneShot(GPTD_, 100);

    for (int i = 0; i < 10; i++) {
      parentThread->sleep(MS2ST(1));

      if (CurrentOperation == owopDone)
        break;
    }

    CurrentOperationValue = nullptr;
    gptStopTimer(GPTD_);

    if (CurrentOperation != owopDone){
      owSend1();
      chprintf((BaseSequentialStream*)&SD1, "wr op error: %d\r\n", CurrentOperation);
      CurrentOperation = owopDone;
      return false;
    }

    return true;
  }

  bool OWDriver::ReadBit(bool *bit) {
    uint8_t b = 0;
    bool res = _Read(&b, 1);
    if (res)
      *bit = (b > 0);
    return res;
  }

  bool OWDriver::Read2Bit(uint8_t *b) {
    bool res = _Read(b, 2);
    if (res)
      *b = *b >> 6;

    return res;
  }

  bool OWDriver::ReadByte(uint8_t *b) {
    return _Read(b, 8);
  }

  bool OWDriver::WriteBit(bool bit) {
    uint8_t b = bit;
    return _Write(&b, 1);
  }

  bool OWDriver::WriteByte(uint8_t b) {
    return _Write(&b, 8);
  }

  OWDriver::OWDriver():  GPTD_{&GPTD4}{
    owList.ClearAll();
  }

  void OWDriver::Init(GPIO_TypeDef *_txPort, uint8_t _txPin, GPIO_TypeDef *_rxPort, uint8_t _rxPin, Rtos::BaseThread *_parentThread) {
    txPort = _txPort;
    txPin  = _txPin;
    rxPort = _rxPort;
    rxPin  = _rxPin;

    parentThread = _parentThread;

    gptStart(GPTD_, &gptconf_);
    GPTD_->customData = static_cast<OWDriver*>(this);
    gptStopTimer(GPTD_);

    palSetPadMode(txPort, txPin, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(rxPort, rxPin, PAL_MODE_INPUT);
    owSend1();
    //debug
    palSetPadMode(GPIOB, 4, PAL_MODE_OUTPUT_PUSHPULL);
    palClearPad(GPIOB, 4);
  }

  bool OWDriver::Ready() {
    return (CurrentOperation == owopDone) && (owRead() == PAL_HIGH);
  }

  bool OWDriver::SendCommandAllNet(uint8_t cmd) {
    bool haveDevice = false;
    if (!Reset(&haveDevice))
        return false;

    if (!haveDevice)
      return true;

    bool res = SendCommand((uint8_t)Command::SkipRom);
    if(!res)
      return res;

    return SendCommand(cmd);
  }

  bool OWDriver::SendCommand(uint8_t cmd) {
    return WriteByte(cmd);
  }

  bool OWDriver::Select(uint8_t *id) {
    bool haveDevice = false;
    if (!Reset(&haveDevice))
        return false;

    if (!haveDevice)
      return true;

    bool res = SendCommand((uint8_t)Command::MatchRom);
    if(!res)
      return res;

    res = Write(id, 8);
    if(!res)
      return res;

    return true;
  }

  bool OWDriver::Write(uint8_t *data, int dataLen) {
    uint8_t vdata[20];
    memcpy(vdata, data, (size_t)dataLen);
    return _Write(vdata, 8 * dataLen);
  }

  bool OWDriver::Read(uint8_t *data, int dataLen) {
    return _Read(data, dataLen * 8);
  }

  bool OWDriver::Search() {
    if (!Ready())
      return false;

    // repeat 0x03 error point (hardware error or search algorithm mistake)
    int RepeatCount = 3;

repeat:
    if (--RepeatCount <= 0)
      return false;

    owList.ClearAll();

    bool haveDevice = false;
    if (!Reset(&haveDevice))
        return false;

    //log("reset: %s\r\n", haveDevice ? "ok":"no devices");

    if (!haveDevice)
      return true;

    uint8_t lastCollision = 0;
    uint8_t prevRom[8] = {0};
    uint8_t currRom[8] = {0};

    for(uint8_t cycles = 0; cycles < 30; cycles++) {

      // reset
      bool haveDevice = false;
      if (!Reset(&haveDevice))
          return false;

      // send search command
      SendCommand((uint8_t)Command::SearchRom);

      // reading one ID
      uint8_t lastZero = 0;
      uint8_t bitIndex = 1;	//Current bit search, start at 1, end at 64 (for convenience)
      uint8_t byteIndex = 0; //Current byte search in
      uint8_t byteMask = 1;
      bool searchDirection = false;
      do {
        uint8_t b;
        Read2Bit(&b);
        bool idBit = b & 1;
        bool idBitComp = b & 2;
        //chprintf((BaseSequentialStream*)&SD1, "read[%02d / %d]: %s %s", bitIndex, byteIndex, (idBit) ? "+":"-", (idBitComp) ? "+":"-");

        // no devices on 1-wire. error or search algorithm mistake...
        if (b == 0x03) {
          //chprintf((BaseSequentialStream*)&SD1, "\r\n 0x03 error!\r\n");

          // make repeat several times...
          goto repeat;
        }

        // All devices coupled have 0 or 1
        if(idBit != idBitComp)
          searchDirection = idBit;  // bit write value for search
        else	//collision here
        {
          // if this discrepancy is before the Last Discrepancy
          // on a previous next then pick the same as last time
          if(bitIndex < lastCollision)
            searchDirection = ((prevRom[byteIndex] & byteMask) > 0);
          else
            searchDirection = (bitIndex == lastCollision);
          if(!searchDirection)
          {
            lastZero = bitIndex;
          }
        }

        if(searchDirection)
          currRom[byteIndex] |= byteMask;
        else
          currRom[byteIndex] &= ~byteMask;

        //chprintf((BaseSequentialStream*)&SD1, " wr: %s %s\r\n", (searchDirection) ? "+":"-", b == 0?"<--":"");
        WriteBit(searchDirection);

        ++bitIndex;
        byteMask <<= 1;

        if(!byteMask)
        {
          ++byteIndex;
          byteMask = 1;
        }
      } while (byteIndex < 8);

      uint8_t crc = crc8_ow(currRom, 8);
      if(crc) {
        log("CRC error: %02x\r\n", crc);
        continue;
      } else {
        //chprintf((BaseSequentialStream*)&SD1, "CRC OK\r\n");
        owList.AddElm(currRom);
      }

      //log("id: %02x %02x %02x %02x %02x %02x %02x %02x\r\n",
      //         currRom[0], currRom[1],currRom[2],currRom[3],currRom[4],currRom[5],currRom[6],currRom[7]);

      // there are no devices more
      if(!(lastCollision = lastZero)) {
        log("Search done.\r\n");
        return true;
      }

      memcpy(prevRom, currRom, 8);
    };

    return true;
  }

  OWList *OWDriver::getOwList() {
    return &owList;
  }

  OWDriver owDriver;
}
