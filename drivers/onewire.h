

#ifndef ONEWIRE_H_
#define ONEWIRE_H_

#include <stdint.h>
#include <stdbool.h>
#include "ch_extended.h"
#include "hal.h"
#include "hal_pal.h"
#include "owlist.h"

namespace OWire {

enum class Command {
  NOP,
  SearchRom = 0xF0,
  ReadRom = 0x33,
  MatchRom = 0x55,
  SkipRom = 0xCC
};

#define OW_SEARCH_FALSE			 0
#define OW_SEARCH_TRUE			 1

#define OW_SEARCH_DONE_NONE	 0
#define OW_SEARCH_DONE		   1
#define OW_SEARCH_DONE_LAST	 2

  class OWDriver {
  private:
    GPTDriver* const GPTD_;
    Rtos::BaseThread *parentThread;
    OWList owList;

    bool Reset(bool *noNetwork);

    bool _Read(uint8_t *bits, int bitCount);
    bool _Write(uint8_t *bits, int bitCount);

    bool ReadBit(bool *bit);
    bool Read2Bit(uint8_t *b);
    bool WriteBit(bool bit);
    bool ReadByte(uint8_t *bit);
    bool WriteByte(uint8_t bit);
  public:
    OWDriver();
    void Init(GPIO_TypeDef *_txPort, uint8_t _txPin, GPIO_TypeDef *_rxPort, uint8_t _rxPin, Rtos::BaseThread *_parentThread);
    void Process();
    bool Ready();

    bool Write(uint8_t data, size_t dataLen);
    bool Read(uint8_t data, size_t maxDataLen, size_t *dataLen);

    bool Search();
    OWList *getOwList();
  };

  extern OWDriver owDriver;
}

#endif /* ONEWIRE_H_ */
