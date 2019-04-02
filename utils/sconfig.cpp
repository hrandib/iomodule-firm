#include "sconfig.h"
#include <string.h>
#include "chprintf.h"
#include "crc8.h"
#include "at24_impl.h"
#include "digitalout.h"

namespace Util {
  SConfig sConfig;

  SConfig::SConfig() {
    Clear();
  }

  void SConfig::Clear() {
#if BOARD_VER == 1
    intConfig.ExecutorEnable = false;
#else
    intConfig.ExecutorEnable = true;
#endif
    intConfig.OWEnable = false;
    intConfig.TempControlEnable = false;
    intConfig.ModbusAddress = 10;

    intConfig.crc = crc8_ow((uint8_t *)&intConfig, sizeof(intConfig) - 1);
  }

  bool SConfig::Init() {
    Clear();
    return LoadFromEEPROM();
  }

  bool SConfig::LoadFromEEPROM() {
    SConfigStruct_t cfg;
    if(sizeof(cfg) != nvram::eeprom.Read(nvram::Section::Setup, cfg)) {
      return false;
    }
    if(crc8_ow((uint8_t *)&intConfig, sizeof(intConfig)))
      return false;

    memcpy(&intConfig, &cfg, sizeof(intConfig));

    return true;
  }

  bool SConfig::SaveToEEPROM() {
    intConfig.crc = crc8_ow((uint8_t *)(&intConfig), sizeof(intConfig) - 1);

    if(sizeof(intConfig) != nvram::eeprom.Write(nvram::Section::Setup, intConfig)) {
      return false;
    }

    return true;
  }

  bool SConfig::GetExecutorEnable() {
    return intConfig.ExecutorEnable;
  }

  void SConfig::SetExecutorEnable(bool exen) {
    intConfig.ExecutorEnable = exen;
  }

  bool SConfig::GetOWEnable() {
    return intConfig.OWEnable;
  }

  void SConfig::SetOWEnable(bool owen) {
    intConfig.OWEnable = owen;
  }

  bool SConfig::GetTempControlEnable() {
    return intConfig.TempControlEnable;
  }

  void SConfig::SetTempControlEnable(bool tempen) {
    intConfig.TempControlEnable = tempen;
  }

  uint16_t SConfig::GetModbusAddress() {
    return intConfig.ModbusAddress;
  }

  void SConfig::SetModbusAddress(uint8_t address) {
    intConfig.ModbusAddress = address;
  }

  uint16_t SConfig::GetConfigWord() {
    return ( (intConfig.ExecutorEnable ? 0x01 : 0x00) |
             (intConfig.OWEnable ? 0x02 : 0x00) |
             (intConfig.TempControlEnable ? 0x04 : 0x00)
             );
  }

  void SConfig::SetConfigWord(uint16_t w) {
    intConfig.ExecutorEnable = w & 0x01;
    intConfig.OWEnable = w & 0x02;
    intConfig.TempControlEnable = w & 0x04;
    CheckDependencies();
  }

  void SConfig::CheckDependencies() {
    if (intConfig.TempControlEnable && intConfig.ExecutorEnable) {
      intConfig.TempControlEnable = false;

      // clear output port...
      Digital::OutputCommand cmd{};
      cmd.Set(decltype(cmd)::Mode::Clear, 0x1F);
      Digital::output.SendMessage(cmd);

      chprintf((BaseSequentialStream*)&SD1, "Temp control ON and executor ON. Set temp control to disable.\r\n");
    }

    if (intConfig.TempControlEnable && !intConfig.OWEnable) {
      intConfig.OWEnable = true;
      chprintf((BaseSequentialStream*)&SD1, "Temp control ON but one wire OFF. Set one wire to enable.\r\n");
    }
  }

  bool SConfig::Print(BaseSequentialStream *chp) {
     chprintf(chp, "config:\r\n");

     chprintf(chp, "executor           : %s\r\n", intConfig.ExecutorEnable ? "enable" : "disable");
     chprintf(chp, "one wire           : %s\r\n", intConfig.OWEnable ? "enable" : "disable");
     chprintf(chp, "temperature control: %s\r\n", intConfig.TempControlEnable ? "enable" : "disable");
     chprintf(chp, "modbus address     : %d\r\n", intConfig.ModbusAddress);

     return true;
  }


}
