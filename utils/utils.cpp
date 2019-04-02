
#include "utils.h"

namespace Util {
void PrintBin(uint32_t data, uint8_t bitCount, uint8_t spaces) {
  for(uint8_t i = 0; i < bitCount; i++) {
    log("%s", (data & (1 << (bitCount - i - 1))) ? "1" : "0");
    if (spaces && (i != bitCount - 1) && (((i + 1) % spaces) == 0))
      log(" ");
  }
}

}

