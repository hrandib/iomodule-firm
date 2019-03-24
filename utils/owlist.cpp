#include "owlist.h"
#include <string.h>
#include "chprintf.h"

namespace OWire {

  static const uint8_t idzero[8] = {0};

  OWListElm *OWList::FindElm(uint8_t *id) {
    for (int i = 0; i < MaxListCount; i++) {
      if (!memcmp(intlist[i].ID, id, 8)) {
          return &intlist[i];
      }
    }

    return nullptr;
  }

  OWListElm *OWList::FindOrAddElm(uint8_t *id) {
    auto elm = FindElm(id);
    if (!elm) {
      for (int i = 0; i < MaxListCount; i++) {
        if (!memcmp(intlist[i].ID, idzero, 8)) {
            memcpy(intlist[i].ID, id, 8);
            return &intlist[i];
        }
      }

      return nullptr;
    }

    return elm;
  }

  OWList::OWList() {
    ClearAll();
  }

  void OWList::ClearAll() {
    memset(intlist, 0x00, sizeof(intlist));
  }

  bool OWList::AddElm(uint8_t *id) {
    return (FindOrAddElm(id) != nullptr);
  }

  bool OWList::SetTemperature(uint8_t *id, uint16_t temperature) {

  }

  bool OWList::SetHumidity(uint8_t *id, uint16_t humidity) {

  }

  uint8_t *OWList::GetModbusMem(uint16_t address, uint16_t size) {
  }

  uint8_t *OWList::GetOWIDByListPosition(int listPos) {
    if (listPos >= MaxListCount)
      return nullptr;

    return intlist[listPos].ID;
  }

  bool OWList::Print(BaseSequentialStream *chp, bool printIDOnly) {

    for (int i = 0; i < MaxListCount; i++) {
      if (!memcmp(intlist[i].ID, idzero, 8)) {
        if(i == 0)
          chprintf(chp, "list is empty\r\n");
        return true;
      }

      chprintf(chp, "ID[%d]: %02x", i, intlist[i].ID[0]);
      if (!printIDOnly) {
        chprintf(chp, " temp: %04x", i, intlist[i].Temperature);
        chprintf(chp, " hum: %04x", i, intlist[i].Humidity);
      }
      chprintf(chp, "\r\n");

    }

  }

}
