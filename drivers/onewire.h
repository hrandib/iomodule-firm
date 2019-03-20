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
#include "gpio.h"

namespace OWire {
/*
  #define OW_SEARCH_ROM			0xF0
  #define OW_READ_ROM				0x33
  #define OW_MATCH_ROM			0x55
  #define OW_SKIP_ROM				0xCC

  #define OW_SEARCH_FALSE			 0
  #define OW_SEARCH_TRUE			 1

  #define OW_SEARCH_DONE_NONE	 0
  #define OW_SEARCH_DONE		   1
  #define OW_SEARCH_DONE_LAST	 2

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

  class OWDriver {
  private:
    bool Reset();
    bool ReadBit(bool *bit);
    bool Read2Bit(uint8_t *bit);
    bool WriteBit(bool bit);
    bool ReadByte(uint8_t *bit);
    bool WriteByte(uint8_t bit);

    bool SearchStart();
    bool SearchNext();
    bool SearchDone();
    bool SearchResult();
  public:
    void Init(TPin rxPin, TPin txPin);
    void Process();
    bool Ready();

    bool Write(uint8_t data, size_t dataLen);
    bool Read(uint8_t data, size_t maxDataLen, size_t *dataLen);

    bool Search();
  };

  extern OWDriver owDriver;
}

#endif /* ONEWIRE_H_ */
