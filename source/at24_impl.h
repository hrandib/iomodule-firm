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

#include "mtd_24aa.hpp"
#include "chprintf.h"

namespace nvram {

template<typename... Ts>
static inline void log(const char* str, Ts... ts) {
  chprintf((BaseSequentialStream*)&SD1, str, ts...);
}

enum class Section {
  Reserved,
  Setup,
};

namespace CAT24C08 {
  enum {
    ADDRESS = 0xA0 >> 1,
    WRITETIME = 5,
    PAGES = 64,
    PAGESIZE = 16,
    ADDR_LEN = 1
  };
}

namespace AT24C02 {
  enum {
    ADDRESS = 0xA0 >> 1,
    WRITETIME = 5,
    PAGES = 32,
    PAGESIZE = 8,
    ADDR_LEN = 1
  };
}

static const I2CConfig i2cfg1 = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2
};

class Eeprom
{
private:
  static constexpr size_t SectionSize = 32;
  Mtd24aa& dev_;
public:
  Eeprom(Mtd24aa& dev) : dev_{dev}
  { }
  void Init() {
    palSetPadMode(GPIOB, 8, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);   /* SCL */
    palSetPadMode(GPIOB, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);   /* SDA */
    i2cStart(&I2CD1, &i2cfg1);
  }
  template<typename T>
  size_t Write(Section sec, const T& obj)
  {
    uint32_t offset = uint32_t(sec) * SectionSize;
    return dev_.write((uint8_t*)&obj, sizeof(obj), offset);
  }
  template<typename T>
  size_t Read(Section sec, T& obj, size_t size = sizeof(T))
  {
    uint32_t offset = uint32_t(sec) * SectionSize;
    return dev_.read((uint8_t*)&obj, size, offset);
  }
};

extern Eeprom eeprom;

} //nvram

#endif // AT24_IMPL_H
