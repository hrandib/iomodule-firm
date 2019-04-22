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

#include "executor_impl.h"
#include "digitalin.h"
#include "digitalout.h"
#include "analogin.h"
#include "order_conv.h"
#include "chprintf.h"
#include "sconfig.h"

#if BOARD_VER == 1
#include "analogout.h"
#endif

#include <array>

using Utils::htons;
using Utils::ntohs;
using Utils::htonl;

Executor executor;

extern "C" {

#define SMALL_RELAY 0x100
#define BIG_RELAY 0x0FF

#define SMALL_RELAY_IMPULSE_LEN 500
#define INPUT_ANTICHATTER 200
#define INPUT_GLOBALOFF 3000

// bits 0, 2, 4, 6 - relay and 8 - small_relay
#define ChRelay2Mask(ch)((1 << ((ch) * 2)) & 0xff)
// bits 1, 3, 5, 7 - triacs
#define ChTriac2Mask(ch)((1 << ((ch) * 2 + 1)) & 0xff)

void Executor::Process()
{
  uint16_t outBuffer16 = Digital::output.GetBinaryVal();
  uint16_t regBuffer16 = Digital::input.GetBinaryVal();
  uint16_t changedBuffer = oldRegBuffer16 ^ regBuffer16;
  systime_t time = chVTGetSystemTimeX();

  // triac control
  if (!TriacsDisabled) {
    for (uint8_t chId = 0; chId < 4; chId++) {
      //bool relayOn = outBuffer16 & ChRelay2Mask(chId);
      bool triacOn = outBuffer16 & ChTriac2Mask(chId);

      // triac=on, timer=off ==> set the timer
      if (triacOn && !triacsTimeOn[chId] && !triacsTimeOff[chId]) {
        triacsTimeOn[chId] = time;
        triacsTimeOff[chId] = 0;
        //Util::log("%d triac %d set time\r\n", time, chId);
        continue;
      }

      // triac=on, timer>100ms ==> toggle the relay, set the timer
      if (triacOn && triacsTimeOn[chId] && time - triacsTimeOn[chId] > 100) {
        IOToggle(ChRelay2Mask(chId));
        triacsTimeOn[chId] = 0;
        triacsTimeOff[chId] = time;
        //Util::log("%d triac %d toggle relay\r\n", time, chId);
        continue;
      }

      // triac=on, 1st phase is done, timer>100ms ==> power off the triac, disable the timer
      if (triacOn && !triacsTimeOn[chId] && time - triacsTimeOff[chId] > 100) {
        IOClear(ChTriac2Mask(chId));
        triacsTimeOn[chId] = 0;
        triacsTimeOff[chId] = 0;
        //Util::log("%d triac %d clear\r\n", time, chId);
        continue;
      }
    }
  }

  // small relay is on and timeout is not set
  if(!pinGOTime && outBuffer16 & SMALL_RELAY) {
    pinGOTime = time;
  }

  // timeout on small relay for 500ms and then put it to high
  if(pinGOTime && time - pinGOTime > SMALL_RELAY_IMPULSE_LEN) {
    IOClear(SMALL_RELAY);
    pinGOTime = 0;
  }

  // timeout ON to OFF control
  for(uint8_t i = 0; i < 16; i++)
    if(PINTimeOffSetup[i]) {
      bool io = outBuffer16 & (1 << i);

      if (!io && PINTimeOff[i])
        PINTimeOff[i] = 0;

      if (io && !PINTimeOff[i])
        PINTimeOff[i] = time;

      // PINTimeOffSetup in 0.1s = 100ms
      if (io && PINTimeOff[i] && time - PINTimeOff[i] > PINTimeOffSetup[i] * 100) {
        IOClear((1 << i) & 0xffff);
        PINTimeOff[i] = 0;
      }
    }

  // main control
  for (uint8_t inputN = 0; inputN < 5; inputN++) {
    uint16_t vbit = uint16_t(1 << inputN);
    // 200ms timeout
    if ((time - pinsTime[inputN] > INPUT_ANTICHATTER) && (changedBuffer & vbit)) {
      if (regBuffer16 & vbit) {
        // clear all if we release AllOFF button or release any button after 3s
        if (inputN > 3 || (time - pinsTime[inputN] > INPUT_GLOBALOFF)) {
          OutClearAll();

          // if it comes not from globalreset input
          if (inputN < 4)
            OutGlobalOff();

          pinGOTime = time;
        } else {
          OutToggle(inputN);
        }
      }

      pinsTime[inputN] = time;
    }
  }

  oldRegBuffer16 = regBuffer16;
  return;
}

bool Executor::IOSet(uint16_t reg) {
  Digital::OutputCommand cmd{};
  cmd.Set(decltype(cmd)::Mode::Set, reg);
  return Digital::output.SendMessage(cmd) == MSG_OK;
}

bool Executor::IOClear(uint16_t reg) {
  Digital::OutputCommand cmd{};
  cmd.Set(decltype(cmd)::Mode::Clear, reg);
  return Digital::output.SendMessage(cmd) == MSG_OK;
}

bool Executor::IOToggle(uint16_t reg) {
  Digital::OutputCommand cmd{};
  cmd.Set(decltype(cmd)::Mode::Toggle, reg);
  return Digital::output.SendMessage(cmd) == MSG_OK;
}

bool Executor::OutSet(uint8_t channel, bool value) {
  if (TriacsDisabled) {
    if (value)   // bits 0, 2, 4, 6 - relay
      return IOSet(ChRelay2Mask(channel));
    else
      return IOClear(ChRelay2Mask(channel));
  } else {
    uint16_t outBuffer16 = Digital::output.GetBinaryVal();

    // set triac timer
    if((outBuffer16 & ChRelay2Mask(channel)) != value) {
      bool res =  IOSet(ChTriac2Mask(channel));
      triacsTimeOn[channel] = chVTGetSystemTimeX();
      return res;
    }
    return true;
  }
}

bool Executor::OutToggle(uint8_t channel) {
  if (TriacsDisabled) {
    return IOToggle(ChRelay2Mask(channel));
  } else {
    bool res =  IOSet(ChTriac2Mask(channel));

    // set triac timer
    triacsTimeOn[channel] = chVTGetSystemTimeX();
    return res;
  }
}

bool Executor::OutClearAll() {
  if (TriacsDisabled) {
    return IOClear(BIG_RELAY);
  } else {
    uint16_t outBuffer16 = Digital::output.GetBinaryVal();

    // shifts relays in ON state to triacs
    bool res =  IOSet(((outBuffer16 & 0x55) << 1) & 0xff);

    // set triac timer
    for (uint8_t i = 0; i < 4; i++)
      if(outBuffer16 & ChRelay2Mask(i))
        triacsTimeOn[i] = chVTGetSystemTimeX();

    return res;
  }
}

bool Executor::OutGlobalOff() {
  return IOSet(SMALL_RELAY);
}

void Executor::Init()
{
  oldRegBuffer16 = 0x1ff;
  TriacsDisabled = false;
  start(NORMALPRIO + 12);
}

void Executor::main()
{
  setName("Executor");
  sleep(MS2ST(3000));

  while(true) {
    bool executorEnable = Util::sConfig.GetExecutorEnable();
    if (executorEnable)
      Process();

    // fast only if we needs triac control
    sleep((TriacsDisabled || !executorEnable) ? MS2ST(100) : MS2ST(20));
  }
}

void Executor::SetTriacsDisabled(bool state) {
  TriacsDisabled = state;
  // TODO: save to flash
}

bool Executor::GetTriacsDisabled() {
  return TriacsDisabled;
}

void Executor::SetPinOff(uint8_t channel, uint16_t time) {
  if (channel > 15)
    return;

  PINTimeOffSetup[channel] = time;
}

void Executor::Print() {
  if (!Util::sConfig.GetExecutorEnable()) {
    Util::log("Executor module in DISABLED state\r\n");
    return;
  }
  Util::log("Executor module status:\r\n");

  uint16_t outBuffer16 = Digital::output.GetBinaryVal();
  uint16_t inBuffer16 = Digital::input.GetBinaryVal();

  Util::log("Triacs: %s\r\n", TriacsDisabled ? "off" : "on");

  Util::log("input values: 0x%04x 0b", inBuffer16);
  Util::PrintBin(inBuffer16, 8, 0);
  Util::log("\r\noutput values: 0x%04x 0b", outBuffer16);
  Util::PrintBin(outBuffer16, 9, 0);
  Util::log("\r\n");

  for (uint8_t i = 0; i < 4; i++)
    Util::log("channel%d: %s\r\n", i, (outBuffer16 & (1 << (i * 2))) ? "on" : "off");

  Util::log("globaloff: %s\r\n", (outBuffer16 & 0x100) ? "on" : "off");

  bool atmr = false;
  for (uint8_t i = 0; i < 16; i++)
    if (PINTimeOffSetup[i]) {
      if (!atmr) {
        Util::log("Channel on->off timer:\r\n");
        atmr = true;
      }
      Util::log("[%d] %d\r\n", i + 1, PINTimeOffSetup[i]);
    }

  Util::log("\r\n");
}

} // extern C
