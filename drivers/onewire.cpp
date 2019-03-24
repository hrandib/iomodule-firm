
#include "onewire.h"
//#include "crc8.h"
//#include "csection.h"

//#include "stm32f10x.h"

#include "chprintf.h"
#include <string.h> //for memset
#include "crc8.h"

enum OwOperation {
  owopDone,
  owopReset,
  owopReadBits,
  owopWriteBits,
};

namespace OWire {
#define owSend1() (palClearPad(txPort, txPin))
#define owSend0() (palSetPad(txPort, txPin))
#define owRead() (palReadPad(rxPort, rxPin))

  // Timer

  static enum OwOperation CurrentOperation = owopDone;
  static uint8_t CurrentOperationPhase = 0;
  static uint8_t CurrentOperationValue = 0;
  static uint8_t CurrentOperationValueBitCnt = 0;

  // global declarations for use in class and timer callback
  static GPIO_TypeDef *txPort;
  static uint8_t txPin;
  static GPIO_TypeDef *rxPort;
  static uint8_t rxPin;

  void TimerDisable(GPTDriver* gpt) {
    chSysLockFromISR();
    gptStopTimerI(gpt);
    chSysUnlockFromISR();
  };

  void TimerOneShot(GPTDriver* gpt, gptcnt_t interval) {
    chSysLockFromISR();
    gptStartOneShotI(gpt, interval);
    chSysUnlockFromISR();
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
            CurrentOperationValue = 0;
            CurrentOperationPhase++;
            TimerOneShot(gpt, 100);
            break;
          case 1:
            CurrentOperationValue = owRead();
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
            if (CurrentOperationValueBitCnt) {
              owSend0();
              CurrentOperationPhase++;
              TimerOneShot(gpt, 1);
              return;
            } else {
              CurrentOperation = owopDone;
              break;
            };
          case 1:
            owSend1();
            CurrentOperationPhase++;
            TimerOneShot(gpt, 2);
            return;
          case 2:
            CurrentOperationValueBitCnt--;
            // reading window - (1mks after `owSend1()`) .. (15mks from start(`owSend0()`))
            bool bit = owRead() & 0x01;
            CurrentOperationValue = (CurrentOperationValue << 1) & 0xff;
            CurrentOperationValue |= bit & 0x01;
            CurrentOperationPhase = 0;
//            palTogglePad(GPIOB, 4);
            TimerOneShot(gpt, 60); // min 60 mks from `owSend1()`
            return;
        }
      break;

      case owopWriteBits:
        switch (CurrentOperationPhase) {
          case 0:
            if (CurrentOperationValueBitCnt) {
              owSend0();
              CurrentOperationPhase++;
              palTogglePad(GPIOB, 4);
              TimerOneShot(gpt, 1);
              return;
            } else {
              CurrentOperation = owopDone;
              break;
            };
          case 1:
            CurrentOperationValueBitCnt--;
            if (CurrentOperationValue & 0x01)
              owSend1();
            CurrentOperationValue = CurrentOperationValue >> 1;
            CurrentOperationPhase++;
            TimerOneShot(gpt, 50);
            return;
          case 2:
            owSend1();
            CurrentOperationPhase = 0;
            palTogglePad(GPIOB, 4);
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

    *networkHaveDevice = (CurrentOperationValue == 0);

    return true;
  }

  bool OWDriver::ReadBit(bool *bit) {
    *bit = false;
    if (owRead() != PAL_HIGH)
      return false;

    CurrentOperation = owopReadBits;
    CurrentOperationPhase = 0;
    CurrentOperationValue = 0;
    CurrentOperationValueBitCnt = 1;
    gptStartOneShot(GPTD_, 1);

    for (int i = 0; i < 10; i++) {
      parentThread->sleep(MS2ST(1)); //ms
      if (CurrentOperation == owopDone)
        break;
    }

    if (CurrentOperation != owopDone){
      owSend1();
      gptStopTimer(GPTD_);
      CurrentOperation = owopDone;
      return false;
    }

    *bit = (CurrentOperationValue > 0);

    return true;
  }

  bool OWDriver::Read2Bit(uint8_t *b) {
    *b = 0;
    if (owRead() != PAL_HIGH)
      return false;

    CurrentOperation = owopReadBits;
    CurrentOperationPhase = 0;
    CurrentOperationValue = 0;
    CurrentOperationValueBitCnt = 2;
    gptStartOneShot(GPTD_, 1);

    for (int i = 0; i < 10; i++) {
      parentThread->sleep(MS2ST(1)); //ms
      if (CurrentOperation == owopDone)
        break;
    }

    if (CurrentOperation != owopDone){
      owSend1();
      gptStopTimer(GPTD_);
      CurrentOperation = owopDone;
      return false;
    }

    *b = CurrentOperationValue;

    return true;
  }

  bool OWDriver::WriteBit(bool bit) {
    if (owRead() != PAL_HIGH)
      return false;

    CurrentOperation = owopWriteBits;
    CurrentOperationPhase = 0;
    CurrentOperationValue = bit;
    CurrentOperationValueBitCnt = 1;
    gptStartOneShot(GPTD_, 1);

    for (int i = 0; i < 10; i++) {
      parentThread->sleep(MS2ST(1)); //ms
      if (CurrentOperation == owopDone)
        break;
    }

    if (CurrentOperation != owopDone){
      owSend1();
      gptStopTimer(GPTD_);
      chprintf((BaseSequentialStream*)&SD1, "wr op error: %d\r\n", CurrentOperation);
      CurrentOperation = owopDone;
      return false;
    }

    return true;
  }

  bool OWDriver::ReadByte(uint8_t *b) {
    *b = 0;
    if (owRead() != PAL_HIGH)
      return false;

    CurrentOperation = owopReadBits;
    CurrentOperationPhase = 0;
    CurrentOperationValue = 0;
    CurrentOperationValueBitCnt = 8;
    gptStartOneShot(GPTD_, 1);

    for (int i = 0; i < 10; i++) {
      parentThread->sleep(MS2ST(1)); //ms
      if (CurrentOperation == owopDone)
        break;
    }

    if (CurrentOperation != owopDone){
      owSend1();
      gptStopTimer(GPTD_);
      CurrentOperation = owopDone;
      return false;
    }

    *b = CurrentOperationValue;

    return true;
  }

  bool OWDriver::WriteByte(uint8_t b) {
    if (owRead() != PAL_HIGH)
      return false;

    CurrentOperation = owopWriteBits;
    CurrentOperationPhase = 0;
    CurrentOperationValue = b;
    CurrentOperationValueBitCnt = 8;
    gptStartOneShot(GPTD_, 1);

    // 1 bit = 60mks, 1 byte = 480mks
    for (int i = 0; i < 10; i++) {
      parentThread->sleep(MS2ST(1)); //ms
      if (CurrentOperation == owopDone)
        break;
    }

    if (CurrentOperation != owopDone){
      owSend1();
      gptStopTimer(GPTD_);
      CurrentOperation = owopDone;
      return false;
    }

    return true;
  }

  bool OWDriver::SearchStart(bool needReset) {
    if (needReset) {
      bool haveDevice = false;
      if (!Reset(&haveDevice))
          return false;

      if (!haveDevice)
        return true;
    }


    return true;
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

  bool OWDriver::Search() {
    if (!Ready())
      return false;

    bool haveDevice = false;
    if (!Reset(&haveDevice))
        return false;

    chprintf((BaseSequentialStream*)&SD1, "reset: %s\r\n", haveDevice ? "ok":"no devices");

    if (!haveDevice)
      return true;

    uint8_t lastCollision = 0;
    uint8_t prevRom[8] = {0};
    uint8_t romArr[8] = {0};
    uint8_t* rom = &romArr[0]; //8 byte buffer for current search

    WriteByte((uint8_t)Command::SearchRom);

    uint8_t lastZero = 0;
    uint8_t bitIndex = 1;	//Current bit search, start at 1, end at 64 (for convenience)
    uint8_t byteIndex = 0; //Current byte search in
    uint8_t byteMask = 1;
    bool searchDirection = false;
    do {
      uint8_t b;
      Read2Bit(&b);
      bool idBit = b & 2;
      bool idBitComp = b & 1;
      chprintf((BaseSequentialStream*)&SD1, "read[%d / %d]: %s %s", bitIndex, byteIndex, (idBit) ? "+":"-", (idBitComp) ? "+":"-");

      // no devices on 1-wire. error or search algorithm mistake...
      if (b == 0x03) {
        chprintf((BaseSequentialStream*)&SD1, "\r\n 0x03 error!\r\n");
        return false;
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
        rom[byteIndex] |= byteMask;
      else
        rom[byteIndex] &= ~byteMask;

      chprintf((BaseSequentialStream*)&SD1, " wr: %s\r\n", (searchDirection) ? "+":"-");
      WriteBit(searchDirection);

      ++bitIndex;
      byteMask <<= 1;

      if(!byteMask)
      {
//        crc(rom[byteIndex]);  // accumulate the CRC
        ++byteIndex;
        byteMask = 1;
      }
    } while (byteIndex < 8);

    uint8_t crc = crc8_ow(romArr, 8);
    if(crc)
      chprintf((BaseSequentialStream*)&SD1, "CRC error: %02x\r\n", crc);
    else
      chprintf((BaseSequentialStream*)&SD1, "CRC OK\r\n");

    chprintf((BaseSequentialStream*)&SD1, "id: %02x %02x %02x %02x %02x %02x %02x %02x\r\n",
             romArr[0], romArr[1],romArr[2],romArr[3],romArr[4],romArr[5],romArr[6],romArr[7]);
    return true;
  }

  OWList *OWDriver::getOwList() {
    return &owList;
  }

  OWDriver owDriver;

}
