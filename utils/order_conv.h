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
#ifndef ORDER_CONV_H
#define ORDER_CONV_H

#include <stdint.h>

namespace Utils {
#define DECLARE_ORDER_CONV(suffix, bits)                        \
  constexpr uint ## bits ## _t hton ## suffix(uint ## bits ## _t val) {  \
    if(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) {             \
      return __builtin_bswap ## bits(val);                      \
    }                                                           \
    else {                                                      \
      return val;                                               \
    }                                                           \
  }                                                             \
  constexpr uint ## bits ## _t ntoh ## suffix(uint ## bits ## _t val) {  \
    return hton ## suffix(val);                                                            \
  }

  DECLARE_ORDER_CONV(s, 16)
  DECLARE_ORDER_CONV(l, 32)
  DECLARE_ORDER_CONV(ll, 64)

} //Utils

#endif // ORDER_CONV_H
