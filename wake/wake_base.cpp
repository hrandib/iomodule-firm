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

#include "wake_base.h"

namespace Wk {

  using namespace Rtos;

  /*
   * This callback is invoked on a receive error, the errors mask is passed
   * as parameter.
   */
  void WakeBase::RXerr(UARTDriver* uartp, uartflags_t)
  {
    WakeBase& wake = *reinterpret_cast<WakeBase*>(uartp->customData);
    wake.state_ = WAIT_FEND;
  }

  /*
   * This callback is invoked when a receive buffer has been completely written.
   */
  void WakeBase::RXend(UARTDriver*)
  {  }

  /*
   * This callback is invoked when a character is received but the application
   * was not ready to receive it, the character is passed as parameter.
   */
  void WakeBase::RXchar(UARTDriver* uartp, uint16_t dataByte_)
  {
    WakeBase& wake = *reinterpret_cast<WakeBase*>(uartp->customData);
    uint8_t dataByte = static_cast<uint8_t>(dataByte_);
    if(dataByte == FEND) {
      wake.prevByte_ = dataByte;
      wake.crc_.Reset(CRC_INIT);
      wake.state_ = ADDR;
      wake.crc_(dataByte);
      return;
    }
    if(wake.state_ == WAIT_FEND) {
      return;
    }
    uint8_t prev = wake.prevByte_;               //сохранение старого пре-байта
    wake.prevByte_ = dataByte;              //обновление пре-байта
    if(prev == FESC) {
      if(dataByte == TFESC)            //а байт данных равен TFESC,
        dataByte = FESC;               //то заменить его на FESC
      else if(dataByte == TFEND)       //если байт данных равен TFEND,
        dataByte = FEND;          //то заменить его на FEND
      else {
        wake.state_ = WAIT_FEND;     //для всех других значений байта данных,
//          cmd = C_ERR;         //следующего за FESC, ошибка
        return;
      }
    }
    else if(dataByte == FESC) {            //если байт данных равен FESC, он просто
      return;                             //запоминается в пре-байте
    }
    switch(wake.state_) {
    case ADDR:                     //-----> ожидание приема адреса
      if(dataByte & 0x80) {
        wake.crc_(dataByte); //то обновление CRC и
        dataByte &= 0x7F; //обнуляем бит 7, получаем истинный адрес
        if(dataByte == 0 || dataByte == DEFAULT_ADDRESS/* || dataByte == groupAddr_nv*/) {
          wake.packetData_.addr = dataByte;
          wake.state_ = CMD;       //переходим к приему команды
          break;
        }
        wake.state_ = WAIT_FEND;        //адрес не совпал, ожидание нового пакета
        break;
      }
      else {
        wake.packetData_.addr = 0;	//если бит 7 данных равен нулю, то
      }
      wake.state_ = CMD;					//сразу переходим к приему команды
    case CMD:                      //-----> ожидание приема команды
      if(dataByte & 0x80) {
        wake.state_ = WAIT_FEND;        //если бит 7 не равен нулю,
//          cmd = C_ERR;            //то ошибка
        break;
      }
      wake.packetData_.cmd = dataByte;          //сохранение команды
      wake.crc_(dataByte);				//обновление CRC
      wake.state_ = NBT;           //переходим к приему количества байт
      break;
    case NBT:
      if(dataByte > WAKEDATABUFSIZE) {
        wake.state_ = WAIT_FEND;
//          cmd = C_ERR;		//то ошибка
        break;
      }
      wake.packetData_.payloadSize = dataByte;
      wake.crc_(dataByte);		//обновление CRC
      wake.ptr_ = 0;			//обнуляем указатель данных
      wake.state_ = DATA;		//переходим к приему данных
      break;
    case DATA:
      if(wake.ptr_ < wake.packetData_.payloadSize) {
        wake.packetData_.buf[wake.ptr_++] = dataByte; //то сохранение байта данных,
        wake.crc_(dataByte);  //обновление CRC
        break;
      }
      if(dataByte != wake.crc_.GetResult()) {
        wake.state_ = WAIT_FEND;		//если CRC не совпадает,
//          cmd = C_ERR;			//то ошибка
        break;
      }
      wake.state_ = WAIT_FEND;		//прием пакета завершен,
      wake.stayPoint_.ResumeFromISR(wake.packetData_.cmd);		//загрузка команды на выполнение
      break;
      //warning suppress
    case WAIT_FEND:
      ;
    }
  }

  /*
   * This callback is invoked when a transmission buffer has been completely
   * read by the driver.
   */
  void WakeBase::TXend1(UARTDriver*)
  {  }

  /*
   * This callback is invoked when a transmission has physically completed.
   */
  void WakeBase::TXend2(UARTDriver* uartp)
  {
    ClearDE(uartp);
  }

  size_t WakeBase::FillTxBuf()
  {
    size_t dataIndex = 0;
    uint8_t dataByte;
    crc_.Reset(CRC_INIT);
    for(int32_t i = -4; i <= packetData_.payloadSize; ++i) {
      //this needed only for master
      if(i == -3 && !packetData_.addr) {
        ++i;
      }
      if(i == -4) {         //FEND
        dataByte = FEND;
      }
      else if(i == -3) {	//address
        dataByte = packetData_.addr | 0x80;
      }
      else if(i == -2) {	//command
        dataByte = packetData_.cmd;
      }
      else if(i == -1) {	//payload count
        dataByte = packetData_.payloadSize;
      }
      else if(i == packetData_.payloadSize) { //crc
        dataByte = crc_.GetResult();
      }
      else {
        dataByte = packetData_.buf[i];  //data
      }
      crc_(dataByte);
      if(i > -4) {
        if(dataByte == FEND || dataByte == FESC) {
          txBuf_[dataIndex++] = FESC;
          dataByte = (dataByte == FEND ? TFEND : TFESC);
        }
      }
      txBuf_[dataIndex++] = dataByte;
    }
    return dataIndex;
  }

}//Wake
