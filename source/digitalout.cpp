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

#include "digitalout.h"

namespace Digital {
  const SPIConfig Output::spicfg_ {
    [](SPIDriver* spid) { spiUnselectI(spid); },
    GPIOB,        //strobe port
    14,           //strobe pad
    SPI_CR1_DFF,  //16 bit transfer
    0
  };
//TODO: Fill with correct values
  const Output::pinmap_t Output::pinMap_{{0x0001, 0x0002, 0x0004, 0x0008,
                                          0x0010, 0x0020, 0x0040, 0x0080,
                                          0x0100, 0x0200, 0x0400, 0x0800,
                                          0x1000, 0x2000, 0x4000, 0x8000,
                                         }};

} //Digital
