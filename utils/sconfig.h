#ifndef SCONFIG_H
#define SCONFIG_H

#include <cstddef>
#include <stdint.h>
#include <stdbool.h>
#include "hal.h"

namespace Util {
  typedef struct {
    bool ExecutorEnable : 1;
    bool OWEnable : 1;
    bool TempControlEnable : 1;
    uint8_t : 0; // start new byte
    uint8_t ModbusAddress : 8;

    uint8_t crc : 8;
  } SConfigStruct_t;

  class SConfig {
  private:
    SConfigStruct_t intConfig;
  public:
    SConfig();

    void Clear();
    bool Init();
    bool LoadFromEEPROM();
    bool SaveToEEPROM();

    bool GetExecutorEnable();
    void SetExecutorEnable(bool exen);
    bool GetOWEnable();
    void SetOWEnable(bool owen);
    bool GetTempControlEnable();
    void SetTempControlEnable(bool tempen);
    uint16_t GetModbusAddress();
    void SetModbusAddress(uint8_t address);

    void CheckDependencies();

    bool Print(BaseSequentialStream *chp);
  };

  extern SConfig sConfig;
}

#endif // SCONFIG_H
