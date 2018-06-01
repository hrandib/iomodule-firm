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
#ifndef AT24_IMPL_H
#define AT24_IMPL_H

#define MTD_USE_MUTUAL_EXCLUSION  TRUE

#include "mtd_24aa.hpp"
#include "chprintf.h"


template<typename... Ts>
static inline void log(const char* str, Ts... ts) {
  chprintf((BaseSequentialStream*)&SD1, str, ts...);
}

namespace CAT24C08 {
  enum {
    ADDRESS = 0xA0 >> 1,
    WRITETIME = 20,
    PAGES = 64,
    PAGESIZE = 16,
    ADDR_LEN = 1
  };
}

#define EEPROM_TYPE CAT24C08

class Eeprom
{
private:
public:
  Eeprom();

};

extern nvram::Mtd24aa nvram_mtd;

#endif // AT24_IMPL_H
