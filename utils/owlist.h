#ifndef OWLIST_H
#define OWLIST_H

#include <cstddef>
#include <stdint.h>
#include <stdbool.h>
#include "hal.h"

namespace OWire {
  typedef struct {
    uint8_t ID[8];
    uint16_t Temperature;
    uint16_t Humidity;
  } OWListElm;

  class OWList {
  private:
    static const int MaxListCount = 20;
    OWListElm intlist[MaxListCount];
    OWListElm *FindElm(uint8_t *id);
    OWListElm *FindOrAddElm(uint8_t *id);
  public:
    OWList();

    void ClearAll();
    bool AddElm(uint8_t *id);
    bool SetTemperature(uint8_t *id, uint16_t temperature);
    bool SetHumidity(uint8_t *id, uint16_t humidity);

    uint8_t *GetModbusMem(uint16_t address, uint16_t size);
    uint8_t *GetOWIDByListPosition(int listPos);

    bool Print(BaseSequentialStream *chp, bool printIDOnly);
  };
}

#endif // OWLIST_H
