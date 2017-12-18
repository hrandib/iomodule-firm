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

#ifndef DISPLAY_H
#define DISPLAY_H

#include "ch_extended.h"
#include "ssd1306.h"

extern Rtos::Mailbox<int32_t, 4> dispMsgQueue;

class Display : public Rtos::BaseStaticThread<256>
{
private:
  using Twi = Twis::SoftTwi<Mcudrv::Pb6, Mcudrv::Pb7>;
  using Disp = Mcudrv::ssd1306<Twi, Mcudrv::ssd1306_128x32>;

  void DisplayCurrent(int32_t val)
  {
    char buf[8];
    io::InsertDot(uint16_t(val / 10), 2, (uint8_t*)buf);
    Disp::Putch2X(' ', Mcudrv::Resources::font10x16);
    Disp::Putch2X(buf[0], Mcudrv::Resources::font10x16);
    Disp::Putch2X('.');
    Disp::Puts2X(&buf[2], Mcudrv::Resources::font10x16);
    Disp::Putch2X('A');
  }
  void DisplayBrightness(int32_t val)
  {
    char buf[8];
    if(val < 10) {
      Disp::Putch2X(' ', Mcudrv::Resources::font10x16);
    }
    if(val < 100) {
      Disp::Putch2X(' ', Mcudrv::Resources::font10x16);
    }
    Disp::Putch2X(' ', Mcudrv::Resources::font10x16);
    io::utoa16((uint16_t)val, (uint8_t*)buf);
    Disp::Puts2X(buf, Mcudrv::Resources::font10x16);
    Disp::Puts2X("%");
  }
public:
  void Init()
  {
    Twi::Init();
    sleep(MS2ST(100));
    Disp::Init();
    Disp::Fill();
    Disp::SetContrast(10);
    start(HIGHPRIO - 1);
  }
  void main() final
  {
    int32_t value;
    enum class State {
      DispCurrent,
      DispBrightness
    } prevState{};
    while(true) {
      msg_t result = dispMsgQueue.fetch(&value, S2ST(10));
      Disp::SetXY(0, 0);
      if(result == MSG_OK) {
        if(value < 0) {
          if(prevState != State::DispCurrent) {
            Disp::Fill();
            prevState = State::DispCurrent;
          }
          DisplayCurrent(-value);
        }
        else {
          if(prevState != State::DispBrightness) {
            Disp::Fill();
            prevState = State::DispBrightness;
          }
          DisplayBrightness(value);
        }
      }
      else {
        Disp::Fill();
      }
    }
  }
};

#endif // DISPLAY_H
