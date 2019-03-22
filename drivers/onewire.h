/*
 * onewire.h
 *
 *  Created on: Feb 4, 2019
 *      Author: herman
 */

#ifndef ONEWIRE_H_
#define ONEWIRE_H_

#include <stdint.h>
#include <stdbool.h>
#include "ch_extended.h"
#include "hal.h"
#include "hal_pal.h"

namespace OWire {
/*
  #define OW_HANDLER_STATUS_NONE	0
  #define OW_HANDLER_STATUS_DONE	1
  #define OW_HANDLER_STATUS_ERROR	2

  extern volatile uint32_t ow_timing;

  void ow_init();

  void ow_reset();
  void ow_write_bit();
  void ow_read_bit();
  void ow_read();
  void ow_write_data(uint8_t data);
  uint8_t ow_read_data();

  void ow_handler();
  uint8_t ow_done();
  uint8_t ow_done_status();

  /*
   * SEARCH Algorithm
   *

  uint8_t ow_search_start();
  uint8_t ow_search_next();
  uint8_t ow_search_done();
  uint8_t *ow_search_result();
*/

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

    bool Reset(bool *noNetwork);
    bool ReadBit(bool *bit);
    bool Read2Bit(uint8_t *b);
    bool WriteBit(bool bit);
    bool ReadByte(uint8_t *bit);
    bool WriteByte(uint8_t bit);

    bool SearchStart(bool needReset);
    bool SearchNext();
    bool SearchDone();
    bool SearchResult();
  public:
    OWDriver();
    void Init(GPIO_TypeDef *_txPort, uint8_t _txPin, GPIO_TypeDef *_rxPort, uint8_t _rxPin, Rtos::BaseThread *_parentThread);
    void Process();
    bool Ready();

    bool Write(uint8_t data, size_t dataLen);
    bool Read(uint8_t data, size_t maxDataLen, size_t *dataLen);

    bool Search();
  };

  extern OWDriver owDriver;
}

#endif /* ONEWIRE_H_ */
