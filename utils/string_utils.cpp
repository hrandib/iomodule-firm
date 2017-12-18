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

#include <string.h>
#include "string_utils.h"

namespace io {
  uint8_t* InsertDot(uint16_t value, uint8_t position, uint8_t* buf)
  {
    size_t len = strlen((const char*)utoa16(value, buf));
    if(len <= position) {
      uint8_t offset = static_cast<uint8_t>(position + 2 - len);
      memmove(buf + offset, buf, len + 1);
      buf[0] = '0';
      buf[1] = '.';
      for(uint8_t x = 2; x < offset; ++x) {
        buf[x] = '0';
      }
    }
    else { //length > position
      memmove(buf + len - position + 1, buf + len - position, position + 1);
      buf[len - position] = '.';
    }
    return buf;
  }
}//io


