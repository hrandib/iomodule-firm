
#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>
#include "ch_extended.h"
#include "hal.h"
#include "chprintf.h"

#define Uint16Swap(d)(uint16_t)(((uint16_t)(d) >> 8) + ((uint16_t)(d) << 8))

namespace Util {
  template<typename... Ts>
  static inline void log(const char* str, Ts... ts) {
    chprintf((BaseSequentialStream*)&SD1, str, ts...);
  }
}

#endif /* UTILS_H_ */
