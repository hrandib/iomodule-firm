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

#include "at24_impl.h"

namespace nvram {

enum {
  MTD_WRITE_BUF_SIZE = EEPROM_TYPE::PAGESIZE + 2
};

static const MtdConfig eecfg = {
  MS2ST(EEPROM_TYPE::WRITETIME),
  MS2ST(EEPROM_TYPE::WRITETIME * EEPROM_TYPE::PAGES),
  EEPROM_TYPE::PAGES,
  EEPROM_TYPE::PAGESIZE,
  EEPROM_TYPE::ADDR_LEN,
  nullptr,
  nullptr
};

static uint8_t workbuf[MTD_WRITE_BUF_SIZE];

static Mtd24aa nvram_mtd(eecfg, workbuf, MTD_WRITE_BUF_SIZE, &I2CD1, EEPROM_TYPE::ADDRESS);

Eeprom eeprom{nvram_mtd};

}
