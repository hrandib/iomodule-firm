#ifndef SCONFIG_H
#define SCONFIG_H

#include <cstddef>
#include <stdint.h>
#include <stdbool.h>
#include "hal.h"

namespace Util {
  typedef struct {
    bool ExecutorEnable;
    bool OWEnable;
    bool TempControlEnable;
    uint16_t ModbusAddress;

    uint8_t crc;
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
    void SetModbusAddress(uint16_t address);

    bool Print(BaseSequentialStream *chp);
  };

  extern SConfig sConfig;
}

#endif // SCONFIG_H
