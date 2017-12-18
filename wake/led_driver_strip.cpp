/*
 * Copyright (c) 2017 Dmytro Shestakov
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

#include "led_driver.h"

namespace Wk {

  const PWMConfig LedDriverFeatures::pwmcfg {
    4000000UL,                                    /* 4MHz PWM clock frequency.   */
    4096,                                         /* Initial PWM period 1ms.      */
    nullptr,
    {
      {PWM_OUTPUT_DISABLED, nullptr},
      {PWM_OUTPUT_DISABLED, nullptr},
      {PWM_OUTPUT_ACTIVE_HIGH, nullptr},
      {PWM_OUTPUT_DISABLED, nullptr}
    },
    0,
    0,
#if STM32_PWM_USE_ADVANCED
    0
#endif
  };

  PWMDriver* const LedDriverFeatures::PWMD{ &PWMD3 };

  const uint16_t LedDriverFeatures::LUT[] = {
    0, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 28, 31, 35, 40, 46,
    53, 61, 70, 80, 91, 103, 116, 130, 145, 161, 178, 196, 215, 235, 256,
    278, 301, 325, 350, 376, 403, 431, 460, 490, 521, 553, 586, 620, 655,
    691, 728, 766, 805, 845, 886, 928, 971, 1015, 1060, 1106, 1153, 1201,
    1250, 1300, 1351, 1403, 1456, 1510, 1565, 1621, 1678, 1736, 1795, 1855,
    1916, 1978, 2041, 2105, 2170, 2236, 2303, 2371, 2440, 2510, 2581, 2653,
    2726, 2800, 2875, 2951, 3028, 3106, 3185, 3265, 3346, 3428, 3511, 3595,
    3680, 3766, 3853, 3941, 4096
  };

  const ioportid_t LedDriverFeatures::pwmPort{ GPIOB };
  const uint16_t LedDriverFeatures::pwmPad{ 0 };

} //Wk
