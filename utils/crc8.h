/*
 * crc8.h
 *
 *  Created on: Jan 31, 2019
 *      Author: herman
 */

#ifndef CRC8_H_
#define CRC8_H_

#include <stdint.h>
#include <stdlib.h>

extern "C" {

uint8_t crc8_ow(uint8_t *data, uint8_t size);
uint8_t crc8_ow2(uint8_t crc, uint8_t data);
uint8_t crc8(uint8_t *data, uint8_t size);
uint8_t crc8_single(uint8_t data);

// maxim
uint8_t crc8mx(uint8_t crc, uint8_t *mem, size_t len);

}
#endif /* CRC8_H_ */
