/*
 * Copyright (c) 2014 Dmytro Shestakov
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

#ifndef PINLIST_H
#define PINLIST_H

#include "gpio.h"

namespace Mcudrv {

  namespace _impl {

    template<typename...>
    struct PinlistImplementation;

    template<typename First, typename... Rest>
    struct PinlistImplementation<First, Rest...> {
      template<uint32_t pos = 0>
      static uint32_t ReadODR()
      {
        return ((uint32_t)First::IsSetODR() << pos) | PinlistImplementation<Rest...>::template ReadODR < pos + 1 > ();
      }
      template<uint32_t pos = 0>
      static uint32_t Read()
      {
        return ((uint32_t)First::IsSet() << pos) | PinlistImplementation<Rest...>::template Read < pos + 1 > ();
      }
      static void Write(uint32_t value)
      {
        First::SetOrClear(value & 0x01);
        PinlistImplementation<Rest...>::Write(value >> 1);
      }
      template<GpioModes conf>
      static void SetConfig()
      {
        First::template SetConfig<GpioBase::Cfg(conf)>();
        PinlistImplementation<Rest...>::template SetConfig<conf>();
      }
    };

    template<>
    struct PinlistImplementation<> {
      template<uint32_t pos>
      static uint32_t ReadODR()
      {
        return 0;
      }
      template<uint32_t pos>
      static uint32_t Read()
      {
        return 0;
      }
      static void Write(uint32_t) {}
      template<GpioModes conf>
      static void SetConfig() {}
    };

    using Utils::NumberToMask;

  } //_impl

  template<uint16_t seq>
  struct SequenceOf {
    enum { value = seq };
  };

  template<typename First, typename... Rest>
  struct Pinlist {
    static uint32_t ReadODR()
    {
      return _impl::PinlistImplementation<First, Rest...>::template ReadODR<>();
    }
    static uint32_t Read()
    {
      return _impl::PinlistImplementation<First, Rest...>::template Read<>();
    }
    static void Write(uint32_t value)
    {
      _impl::PinlistImplementation<First, Rest...>::Write(value);
    }
    template<GpioModes conf>
    static void SetConfig()
    {
      _impl::PinlistImplementation<First, Rest...>::template SetConfig<conf>();
    }
  };

  template<typename First, uint16_t Seq>
  struct Pinlist<First, SequenceOf<Seq>> {
    enum {
      offset = First::position,
      mask = _impl::NumberToMask<Seq>::value
    };
    using Port = typename First::Port;
    static uint16_t ReadODR()
    {
      return (Port::ReadODR() >> offset) & mask;
    }
    static uint16_t Read()
    {
      return (Port::Read() >> offset) & mask;
    }
    static void Write(uint16_t value)
    {
      Port::ClearAndSet((~value & mask) << offset, (value & mask) << offset);
    }
    template<GpioModes conf>
    static void SetConfig()
    {
      Port::template SetConfig<mask, GpioBase::Cfg(conf)>();
    }
  };

} //Mcudrv

#endif // PINLIST_H
