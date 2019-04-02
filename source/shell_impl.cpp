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

#include "shell_impl.h"
#include "hal.h"
#include "shell.h"
#include <string.h>
#include "digitalout.h"
#include "analogin.h"
#include "digitalin.h"
#include "modbus_impl.h"
#include "chprintf.h"
#include "string_utils.h"
#include "owmaster_impl.h"
#include "sconfig.h"
#include "tempcontrol_impl.h"

#if BOARD_VER == 1
#include "analogout.h"
#endif

using namespace std::literals;

static THD_WORKING_AREA(SHELL_WA_SIZE, 512);
#if BOARD_VER == 1
static void cmd_setanalog(BaseSequentialStream *chp, int argc, char *argv[]);
#endif
static void cmd_getanalog(BaseSequentialStream *chp, int argc, char *argv[]);
static void cmd_setdigital(BaseSequentialStream *chp, int argc, char *argv[]);
static void cmd_getdigital(BaseSequentialStream *chp, int argc, char *argv[]);
static void cmd_getcounters(BaseSequentialStream *chp, int argc, char *argv[]);
static void cmd_uptime(BaseSequentialStream *chp, int argc, char *argv[]);
static void cmd_setup(BaseSequentialStream *chp, int argc, char *argv[]);
static void cmd_ow(BaseSequentialStream *chp, int argc, char *argv[]);
static void cmd_temp(BaseSequentialStream *chp, int argc, char *argv[]);

static const ShellCommand commands[] = {
#if BOARD_VER == 1
  {"setanalog", cmd_setanalog},
#endif
  {"getanalog", cmd_getanalog},
  {"setdigital", cmd_setdigital},
  {"getdigital", cmd_getdigital},
  {"getcounters", cmd_getcounters},
  {"uptime", cmd_uptime},
  {"setup", cmd_setup},
  {"temp", cmd_temp},
  {"ow", cmd_ow},
  {nullptr, nullptr}
};

static char histbuf[128];

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SD1,
  commands,
  histbuf,
  128
};

#if BOARD_VER == 1
void cmd_setanalog(BaseSequentialStream *chp, int argc, char *argv[])
{
  using namespace Analog;
  OutputCommand cmd{};
  auto PrintValues = [&] {
    chprintf(chp, "%4u %4u %4u %4u\r\n",
             cmd.GetValue(0),
             cmd.GetValue(1),
             cmd.GetValue(2),
             cmd.GetValue(3));
  };
  if(argc == 0) {
    output.SendMessage(cmd);
    PrintValues();
    return;
  }
  if(argc == 2) do {
    pwmchannel_t ch = *argv[0] - '0';
    if(ch > 3) {
      break;
    }
    auto value = io::svtou(argv[1]);
    if(!value || *value > Output::Resolution) {
      break;
    }
    cmd.SetValue(ch, (uint16_t)*value);
    output.SendMessage(cmd);
    PrintValues();
    return;
  } while(false);
  shellUsage(chp, "Set analog output value on selected channel"
                  "\r\nReturns array of current values"
                  "\r\n\tsetanalog [channel(0-3)] [value(0-4096)]"
                  "\r\nExample:"
                  "\r\n\tsetanalog 1 2048");
}
#endif

void cmd_getanalog(BaseSequentialStream *chp, int argc, char *argv[])
{
  using namespace Analog;
  static constexpr uint16_t AllChannelsMask = Utils::NumberToMask_v<Analog::Input::numChannels>;
  uint16_t channelMask;
  do {
    if(argc == 0) {
      channelMask = AllChannelsMask;
    }
    else if(argc == 1) {
      auto mask = io::svtou(argv[0]);
      if(mask && *mask > 0 && *mask <= AllChannelsMask) {
        channelMask = static_cast<uint16_t>(*mask);
      }
      else {
        break;
      }
    }
    else {
      break;
    }
    auto samples = Analog::input.GetSamples();
    for(size_t i{}; i < samples.size(); ++i) {
      if(channelMask & (1U << i)) {
        chprintf(chp, "%4u ", samples[i]);
      }
    }
    chprintf(chp, "\r\n");
    return;
  }
  while(false);
  shellUsage(chp, "Get samples array"
                  "\r\nReturns samples on selected channels or all channels data if no arguments passed"
                  "\r\n\tgetanalog [channel mask(0x01-0x3FF)]"
                  "\r\nExamples:"
                  "\r\nGet values of 3rd and 6th channels"
                  "\r\n\tgetanalog 0b100100"
                  "\r\n  or"
                  "\r\n\tgetanalog 0x24");
}

void cmd_getdigital(BaseSequentialStream *chp, int argc, char **/*argv*/)
{
  if(!argc) {
    chprintf(chp, "%x\r\n", Digital::input.GetBinaryVal());
  }
  else {
    shellUsage(chp, "Get value of the digital input register in hex format");
  }
}

void cmd_getcounters(BaseSequentialStream *chp, int argc, char *argv[])
{
  using namespace Digital;
  static constexpr uint16_t AllChannelsMask = Utils::NumberToMask_v<Input::numChannels>;
  uint16_t channelMask;
  do {
    if(argc == 0) {
      channelMask = AllChannelsMask;
    }
    else if(argc == 1) {
      auto mask = io::svtou(argv[0]);
      if(mask && *mask > 0 && *mask <= AllChannelsMask) {
        channelMask = static_cast<uint16_t>(*mask);
      }
      else {
        break;
      }
    }
    else {
      break;
    }
    auto counters = input.GetCounters();
    for(size_t i{}; i < counters.size(); ++i) {
      if(channelMask & (1U << i)) {
        chprintf(chp, "%10u ", counters[i]);
      }
    }
    chprintf(chp, "\r\n");
    return;
  }
  while(false);
  shellUsage(chp, "Get counters array"
                  "\r\nReturns selected counters from the array or all counters if no arguments passed"
                  "\r\n\tgetcounters [channel mask(0x01-0x3FF)]"
                  "\r\nExamples:"
                  "\r\nGet values of 2nd and 5th channels"
                  "\r\n\tgetcounters 0b10010"
                  "\r\n  or"
                  "\r\n\tgetcounters 0x12");
}

void cmd_setdigital(BaseSequentialStream *chp, int argc, char *argv[])
{
  using namespace Digital;
  using Mode = OutputCommand::Mode;
  static constexpr size_t valueMask = Utils::NumberToMask_v<OutputCommand::GetBusWidth()>;
  OutputCommand cmd{};
  do {
    if(argc == 0) {
      output.SendMessage(cmd);
      chprintf(chp, "%x\r\n", cmd.GetValue());
      return;
    }
    else if(argc == 3 && "set_clear"sv == argv[0]) {
      auto setVal = io::svtou(argv[1]);
      if(!setVal || *setVal > valueMask) {
        break;
      }
      auto clearVal = io::svtou(argv[2]);
      if(!clearVal || *clearVal > valueMask) {
        break;
      }
      uint32_t val = *setVal | (*clearVal << OutputCommand::GetBusWidth());
      cmd.Set(Mode::SetAndClear, val);
      output.SendMessage(cmd);
      chprintf(chp, "%x\r\n", cmd.GetValue());
      return;
    }
    else if(argc == 2) {
      if("set"sv == argv[0]) {
        cmd.SetMode(Mode::Set);
      }
      else if("clear"sv == argv[0]) {
        cmd.SetMode(Mode::Clear);
      }
      else if("write"sv == argv[0]) {
        cmd.SetMode(Mode::Write);
      }
      else if("toggle"sv == argv[0]) {
        cmd.SetMode(Mode::Toggle);
      }
      else {
        break;
      }
      auto value = io::svtou(argv[1]);
      if(!value || *value > valueMask) {
        break;
      }
      cmd.SetValue((uint16_t)*value);
      output.SendMessage(cmd);
      chprintf(chp, "%x\r\n", cmd.GetValue());
      return;
    }
  } while(false);
  shellUsage(chp, "Set state of the digital output register"
                  "\r\npossible modes: set, clear, set_clear, write, toggle"
                  "\r\nReturns modified register value"
             //TODO: get mask at compile time
                  "\r\n\tsetdigital [mode] [mask(0-65535)]"
                  "\r\nExamples:"
                  "\r\n\tsetdigital set 0x2020"
                  "\r\n\tsetdigital set_clear 0x0088 0xFF00"
                  "\r\n\tsetdigital toggle 666");
}

void cmd_uptime(BaseSequentialStream *chp, int argc, char **/*argv[]*/)
{
  if(!argc) {
    chprintf(chp, "%u\r\n", uptimeCounter.load());
  }
}

void cmd_setup(BaseSequentialStream *chp, int argc, char* argv[])
{
  bool needHelp = false;
  if(!argc) {
    Util::sConfig.Print(chp);
    return;
  }

  for(int i = 0; i < argc; i++) {
    if (("mbid"sv == argv[i]) && (i + 1 < argc)) {
      i++;
      auto id = io::svtou(argv[i]);
      if(id && *id > 0 && *id < 247) {
        Util::sConfig.SetModbusAddress((uint8_t)*id);

        eMBErrorCode status = modbus.SetID((uint8_t)*id);
        if(status != MB_ENOERR) {
          chprintf(chp, "Set id error: %u\r\n", status);
        }
      } else {
        chprintf(chp, "Modbus address error. Must be 1.. 246. but: %d\r\n", id ? *id : 0);
      }

      continue;
    }

    if ("load"sv == argv[i]) {
      if (!Util::sConfig.LoadFromEEPROM())
        chprintf(chp, "EEPROM load error.\r\n");
      return;
    }

    bool fplus = argv[i][0] == '+';
    bool fminus = argv[i][0] == '-';
    if (fplus || fminus) {
      if ("all"sv == &argv[i][1]) {
        Util::sConfig.SetExecutorEnable(fplus);
        Util::sConfig.SetOWEnable(fplus);
        Util::sConfig.SetTempControlEnable(fplus);
      }
      if ("ex"sv == &argv[i][1]) {
        Util::sConfig.SetExecutorEnable(fplus);
      }
      if ("ow"sv == &argv[i][1]) {
        Util::sConfig.SetOWEnable(fplus);
      }
      if ("temp"sv == &argv[i][1]) {
        Util::sConfig.SetTempControlEnable(fplus);
      }

      continue;
    }

    needHelp = true;
  }

  Util::sConfig.CheckDependencies();
  if (!Util::sConfig.SaveToEEPROM()) {
    chprintf(chp, "EEPROM save error.\r\n");
  };

  if (!needHelp) {
    Util::sConfig.Print(chp);
  } else {
    shellUsage(chp, "Setup module modes"
                    "\r\nReturns current setup if no arguments passed"
                    "\r\nsetup values:"
                    "\r\n\tmbid - modbus id"
                    "\r\n\tex - executor"
                    "\r\n\tow - one wire master"
                    "\r\n\ttemp - temperature controller"
                    "\r\n\tall - all modules"
                    "\r\nexamples:"
                    "\r\n\tow +ex - enable executor"
                    "\r\n\tow -ex - disable executor");
  }

  return;
}

void cmd_ow(BaseSequentialStream *chp, int argc, char* argv[])
{
  if(!argc) {
    chprintf(chp, "ow devices list:\r\n");
    owMaster.SendMessage(owcmdPrintOWList);
    return;
  }

  if("scan"sv == argv[0]) {
    owMaster.SendMessage(owcmdRescanNetwork);
    return;
  }

  if("mes"sv == argv[0]) {
    owMaster.SendMessage(owcmdMeasurement);
    return;
  }

  shellUsage(chp, "Control OneWire master module"
                  "\r\nReturns current devices list if no arguments passed"
                  "\r\n\tow scan - manually rescans network"
                  "\r\n\tow mes - manually start measurement cycle");
  return;
}

void cmd_temp(BaseSequentialStream *chp, int argc, char* argv[])
{
  if(!argc) {
    tempControl.SendMessage(tccmdPrint);
    return;
  }

  if("state"sv == argv[0]) {
    tempControl.SendMessage(tccmdPrintStatus);
    return;
  }

  if("load"sv == argv[0]) {
    // TODO
    return;
  }

  if("save"sv == argv[0]) {
    // TODO
    return;
  }

  if("set"sv == argv[0] && argc >= 4) {
    auto channelN = io::svtou(argv[1]);
    if (!channelN || *channelN < 1 || *channelN > 4) {
      chprintf(chp, "error: channel must be 1..4\r\n");
      return;
    }

    if("id1"sv == argv[2] || "id2"sv == argv[2]) {
      if (strlen(argv[3]) != 16) {
        chprintf(chp, "error: id must be 8 bytes in hex\r\n");
        return;
      }

      uint8_t id[8] = {0};
      if (!io::hextobin(argv[3], id, 8)) {
        chprintf(chp, "error: id not a hex number\r\n");
        return;
      }

      tempControl.SetID((uint8_t)*channelN - 1, ((argv[2][2] == '1') ? 0 : 1), id);

    } else if("temp1"sv == argv[2] || "temp2"sv == argv[2]) {
      auto value = io::svtou(argv[3]);
      if (!value || *value > 0xffff) {
        chprintf(chp, "error: value must be 0..0xffff\r\n");
        return;
      }

      tempControl.SetTemp((uint8_t)*channelN - 1, ((argv[2][4] == '1') ? 0 : 1), (uint16_t)*value);

    } else {
      chprintf(chp, "error: value name must be [id1, id2, temp1, temp2]");
      return;
    }

    return;
  }

  if(("enable"sv == argv[0] || "disable"sv == argv[0]) && argc >= 2) {
    bool en = "enable"sv == argv[0];

    if("all"sv == argv[1]) {
      tempControl.SetChEnable(0, en);
      tempControl.SetChEnable(1, en);
      tempControl.SetChEnable(2, en);
      tempControl.SetChEnable(3, en);

      return;
    }

    auto channelN = io::svtou(argv[1]);
    if (!channelN || *channelN < 1 || *channelN > 4) {
      chprintf(chp, "error: channel must be 1..4 or all");
      return;
    }

    tempControl.SetChEnable((uint8_t)*channelN - 1, en);

    return;
  }

  shellUsage(chp, "Setup and view temperature control module"
                  "\r\nReturns temp control current setup if no arguments passed"
                  "\r\narguments:"
                  "\r\n\tstate - prints status"
                  "\r\n\tload - loads current setup from eeprom"
                  "\r\n\tsave - saves current setup to eeprom"
                  "\r\n\tdisable <channel num> - disable channel"
                  "\r\n\tenable <channel num> - disable channel"
                  "\r\n\tset <channel num> <value name> <value> - set config values"
                  "\r\nchannel num:"
                  "\r\n\t1..4"
                  "\r\nvalue names:"
                  "\r\n\tid1 - 1-wire sensor id for close loop"
                  "\r\n\tid2 - 1-wire sensor id for open loop"
                  "\r\n\temp1 - temperature control for close loop"
                  "\r\n\temp2 - temperature control for open loop"
                  "\r\ntemperature:"
                  "\r\n\ttemperature calc: (temperature + 100) * 100"
                  "\r\n\t25C = 12500"
                  "\r\nexamples:"
                  "\r\n\ttemp enable 2 - enable channel 2"
                  "\r\n\ttemp disable all - disable all channels"
                  "\r\n\ttemp set 2 id1 0102030405060708 - set sensor 1 id for channel 2"
                  "\r\n\ttemp set 3 temp1 12250 - set sensor 1 temperature (22.5C) for channel 3"
                  "\r\n\ttemp mes - manually start measurement cycle");
  return;
}

Shell::Shell()
{
  palSetPadMode(GPIOB, 6, PAL_MODE_STM32_ALTERNATE_PUSHPULL); // tx
  palSetPadMode(GPIOB, 7, PAL_MODE_INPUT); //rx
  SerialConfig sercfg{ 115200,
                       USART_CR1_TE | USART_CR1_RE | USART_CR1_UE,
                           0,
                           0};
  sdStart(&SD1, &sercfg);
  shellInit();
  auto thd = chThdCreateStatic(SHELL_WA_SIZE, sizeof(SHELL_WA_SIZE), NORMALPRIO, shellThread, (void*)&shell_cfg1);
  chRegSetThreadNameX(thd, "Shell");
}
