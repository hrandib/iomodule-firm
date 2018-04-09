/*
 * Copyright (c) 2018 Dmytro Shestakov
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

#include "analogout.h"


namespace Analog {
  const PWMConfig Output::pwmcfg {
    4000000UL,                                    /* 4MHz PWM clock frequency.   */
    4096,                                         /* Initial PWM period 1ms.      */
    nullptr,
    {
      {PWM_OUTPUT_ACTIVE_HIGH, nullptr},
      {PWM_OUTPUT_ACTIVE_HIGH, nullptr},
      {PWM_OUTPUT_ACTIVE_HIGH, nullptr},
      {PWM_OUTPUT_ACTIVE_HIGH, nullptr}
    },
    0,
    0,
#if STM32_PWM_USE_ADVANCED
    0
#endif
  };

  PWMDriver* const Output::PWMD{&PWMD1};
}
