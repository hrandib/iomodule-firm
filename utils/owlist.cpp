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
            intlist[i].Temperature = 0xffff;
            intlist[i].Humidity = 0xffff;
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
    auto elm = FindElm(id);
    if (!elm)
      return false;

    elm->Temperature = temperature;
    return true;
  }

  bool OWList::SetHumidity(uint8_t *id, uint16_t humidity) {
    auto elm = FindElm(id);
    if (!elm)
      return false;

    elm->Humidity = humidity;
    return true;
  }

  uint8_t *OWList::GetModbusMem(uint16_t address, uint16_t size) {
    if (address + size > sizeof(intlist))
      return nullptr;

    return ((uint8_t *)&intlist + address);
  }

  uint8_t *OWList::GetOWIDByListPosition(int listPos) {
    if (listPos >= MaxListCount)
      return nullptr;

    return intlist[listPos].ID;
  }

  bool OWList::isSensorPresent(uint8_t *id) {
    return (FindElm(id) != nullptr);
  }

  uint16_t OWList::GetTemperature(uint8_t *id) {
    OWListElm *elm = FindElm(id);

    if (!elm)
      return 0xffff;

    return elm->Temperature;
  }

  uint16_t OWList::GetHumidity(uint8_t *id) {
    OWListElm *elm = FindElm(id);

    if (!elm)
      return 0xffff;

    return elm->Humidity;
  }

  int OWList::Count() {
    for (int i = 0; i < MaxListCount; i++)
      if (!memcmp(intlist[i].ID, idzero, 8)) {
        return i;
      }

    return MaxListCount;
  }

  bool OWList::Print(BaseSequentialStream *chp, bool printIDOnly) {

    for (int i = 0; i < MaxListCount; i++) {
      if (!memcmp(intlist[i].ID, idzero, 8)) {
        if(i == 0)
          chprintf(chp, "list is empty\r\n");
        return true;
      }

      chprintf(chp, "ID[%d]:", i);
      for (int j = 0; j < 8; j ++)
        chprintf(chp, " %02x", intlist[i].ID[j]);
      if (!printIDOnly) {
        if (intlist[i].Temperature != 0xffff) {
          int t1 = (intlist[i].Temperature) / 100;
          chprintf(chp, " temp: %d.%02d (%d)", t1 - 100, intlist[i].Temperature - t1 * 100, intlist[i].Temperature - 100 * 100);
        } else {
          chprintf(chp, " temp: n/a");
        }

        if (intlist[i].Humidity != 0xffff)
          chprintf(chp, " hum: %d", intlist[i].Humidity - 100 * 100);
        else
          chprintf(chp, " hum: n/a");
      }
      chprintf(chp, "\r\n");
    }

    return true;
  }

}
