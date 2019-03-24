#include "sconfig.h"
#include <string.h>
#include "chprintf.h"
#include "crc8.h"
#include "at24_impl.h"

namespace Util {

  SConfig::SConfig() {
    Clear();
  }

  void SConfig::Clear() {
    intConfig.ExecutorEnable = true;
    intConfig.OWEnable = true;
    intConfig.TempControlEnable = true;
    intConfig.ModbusAddress = 10;

    intConfig.crc = crc8_ow((uint8_t *)&intConfig, sizeof(intConfig) - 1);
  }

  bool SConfig::Init() {
    return LoadFromEEPROM();
  }

  bool SConfig::LoadFromEEPROM() {
    SConfigStruct_t cfg;
    if(sizeof(cfg) != nvram::eeprom.Write(nvram::Section::Modbus, cfg)) {
      return false;
    }
    if(crc8_ow((uint8_t *)&intConfig, sizeof(intConfig)))
      return false;

    return true;
  }

  bool SConfig::SaveToEEPROM() {
    SConfigStruct_t cfg;
    if(sizeof(cfg) != nvram::eeprom.Write(nvram::Section::Modbus, cfg)) {
      return false;
    }
    if(crc8_ow((uint8_t *)&intConfig, sizeof(intConfig)))
      return false;

    memcpy(&intConfig, &cfg, sizeof(intConfig));

    return true;
  }

  bool SConfig::Print(BaseSequentialStream *chp) {
     chprintf(chp, "config:\r\n");

     chprintf(chp, "executor           : %s\r\n", intConfig.ExecutorEnable ? "enable" : "disable");
     chprintf(chp, "one wire           : %s\r\n", intConfig.OWEnable ? "enable" : "disable");
     chprintf(chp, "temperature control: %s\r\n", intConfig.TempControlEnable ? "enable" : "disable");
     chprintf(chp, "modbus address     : %d\r\n", intConfig.ModbusAddress);

     return true;
  }



  SConfig sConfig;
}
