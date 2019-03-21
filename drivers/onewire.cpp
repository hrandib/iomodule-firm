/*
 * onewire.c
 *
 *  Created on: Feb 4, 2019
 *      Author: herman
 */

#include "onewire.h"
//#include "crc8.h"
//#include "csection.h"

//#include "stm32f10x.h"

#include <string.h> //for memset

/*
volatile uint32_t ow_timing 	= 0;
static uint8_t rom_tmp[8] 		= {0};
static uint8_t read_tmp 		= 0;
static uint8_t write_tmp 		= 0;

/*
 *	GPIO CONFIG
 *

#define OW_GPIO_CLOCK			RCC_APB2Periph_GPIOA
#define OW_GPIO_BASE			GPIOA
#define OW_GPIO					GPIO_Pin_0

#define OW_PIN_OUT				OW_GPIO_BASE->CRL = ((OW_GPIO_BASE->CRL & (~(uint32_t)0x0000000F)) | (uint32_t)0x00000007)
#define OW_PIN_IN				OW_GPIO_BASE->CRL = ((OW_GPIO_BASE->CRL & (~(uint32_t)0x0000000F)) | (uint32_t)0x00000004)

#define OW_PIN_HI				OW_GPIO_BASE->BSRR = OW_GPIO
#define OW_PIN_LO				OW_GPIO_BASE->BSRR = (OW_GPIO << 16)

#define OW_PIN_HLVL				((OW_GPIO_BASE->IDR & OW_GPIO) == OW_GPIO)
#define OW_PIN_LLVL				((OW_GPIO_BASE->IDR & OW_GPIO) != OW_GPIO)

/*
 * Basic Delays
 *

#define OW_DELAY480				ow_timing = 48
#define OW_DELAY410				ow_timing = 41
#define OW_DELAY70				ow_timing = 7
#define OW_DELAY60				ow_timing = 6
#define OW_DELAY10				ow_timing = 1

/*
 * Handler states
 *

#define OW_HANDLER_STATE_NONE 			0

#define OW_HANDLER_STATE_RESET			1
#define OW_HANDLER_STATE_RESET_S1		2
#define OW_HANDLER_STATE_RESET_S2		3
#define OW_HANDLER_STATE_RESET_S3		4

#define OW_HANDLER_STATE_READ_BIT		10
#define OW_HANDLER_STATE_READ_BIT_S1	11
#define OW_HANDLER_STATE_READ_BIT_S2	12
#define OW_HANDLER_STATE_READ_BIT_S3	13

#define OW_HANDLER_STATE_WRITE_BIT		20
#define OW_HANDLER_STATE_WRITE_BIT_S1	21
#define OW_HANDLER_STATE_WRITE_BIT_S2	22

#define OW_HANDLER_STATE_READ			30
#define OW_HANDLER_STATE_READ_S1		31

#define OW_HANDLER_STATE_WRITE			40
#define OW_HANDLER_STATE_WRITE_S1		41

#define OW_HANDLER_STATE_DONE			100

static uint8_t handler_state 			= OW_HANDLER_STATE_NONE;
static uint8_t handler_state_return 	= OW_HANDLER_STATE_NONE;
static uint8_t handler_status 			= OW_HANDLER_STATUS_NONE;

/*
 * SEARCH Params
 *

#define OW_SEARCH_STATE_START 						0
#define OW_SEARCH_STATE_NEXT 						1
#define OW_SEARCH_STATE_SEND_SEARCH 				2
#define OW_SEARCH_STATE_SEND_READ_ID_BIT 			3
#define OW_SEARCH_STATE_SEND_READ_CMP_ID_BIT 		4
#define OW_SEARCH_STATE_D 							5
#define OW_SEARCH_STATE_D2 							6
#define OW_SEARCH_STATE_D3 							7

#define OW_SEARCH_STATE_DONE						50

#define OW_SEARCH_STATE_RRW_DONE 					100

static uint8_t search_state 						= OW_SEARCH_STATE_DONE;
static uint8_t search_state_return 					= OW_SEARCH_STATE_DONE;
static uint8_t search_status 						= OW_SEARCH_DONE_NONE;

static uint8_t LastDiscrepancy						= 0;
static uint8_t LastFamilyDiscrepancy				= 0;
static uint8_t LastDeviceFlag						= 0;

static void ow_search_handler();

/*
 *
 *

void ow_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//handler
	ow_timing = 0;
	memset((void*)rom_tmp, 0, sizeof(rom_tmp));
	read_tmp = 0;
	write_tmp = 0;

	handler_state = OW_HANDLER_STATE_NONE;
	handler_state_return = OW_HANDLER_STATE_NONE;
	handler_status = OW_HANDLER_STATUS_NONE;

	//search
	search_state = OW_SEARCH_STATE_DONE;
	search_state_return = OW_SEARCH_STATE_DONE;
	search_status = OW_SEARCH_DONE_NONE;

	LastDiscrepancy = 0;
	LastFamilyDiscrepancy = 0;
	LastDeviceFlag = 0;

	RCC_APB2PeriphClockCmd(OW_GPIO_CLOCK, ENABLE);

	GPIO_InitStructure.GPIO_Pin = OW_GPIO;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(OW_GPIO_BASE, &GPIO_InitStructure);
}

void ow_reset()
{
	handler_state = OW_HANDLER_STATE_RESET;
}

void ow_write_bit()
{
	handler_state = OW_HANDLER_STATE_WRITE_BIT;
	handler_state_return = OW_HANDLER_STATE_DONE;
}

void ow_read_bit()
{
	handler_state = OW_HANDLER_STATE_READ_BIT;
	handler_state_return = OW_HANDLER_STATE_DONE;
}

void ow_write()
{
	handler_state = OW_HANDLER_STATE_WRITE;
}

void ow_read()
{
	handler_state = OW_HANDLER_STATE_READ;
}

void ow_write_data(uint8_t data)
{
	write_tmp = data;
	ow_write();
}

uint8_t ow_read_data()
{
	return read_tmp;
}

uint8_t ow_done()
{
	return handler_state == OW_HANDLER_STATE_DONE;
}

uint8_t ow_done_status()
{
	return handler_status;
}

void ow_handler()
{
	static uint8_t t1 = 0;
	static uint8_t t2 = 0;

	switch (handler_state) {

/*
 * RESET
 *

	case OW_HANDLER_STATE_RESET:
		handler_status = OW_HANDLER_STATUS_NONE;

		OW_PIN_LO;
		OW_PIN_OUT;
		OW_DELAY480;

		handler_state = OW_HANDLER_STATE_RESET_S1;
		break;

	case OW_HANDLER_STATE_RESET_S1:
		if (ow_timing)
			break;

		OW_PIN_IN;

		OW_DELAY70;
		handler_state = OW_HANDLER_STATE_RESET_S2;
		break;

	case OW_HANDLER_STATE_RESET_S2:
		if (ow_timing)
			break;

		if (OW_PIN_LLVL)
			handler_status = OW_HANDLER_STATUS_DONE;

		OW_DELAY410;
		handler_state = OW_HANDLER_STATE_RESET_S3;
		break;

	case OW_HANDLER_STATE_RESET_S3:
		if (ow_timing)
			break;

		handler_state = OW_HANDLER_STATE_DONE;
		break;

/*
 * READ BIT
 *

	case OW_HANDLER_STATE_READ_BIT:
		CSECTION_LOCK(CSECTION_OW);
		OW_PIN_LO;
		OW_PIN_OUT;

		OW_DELAY10;

		handler_state = OW_HANDLER_STATE_READ_BIT_S1;
		break;

	case OW_HANDLER_STATE_READ_BIT_S1:
		if (ow_timing)
			break;

		OW_PIN_IN;

		OW_DELAY10;
		handler_state = OW_HANDLER_STATE_READ_BIT_S2;
		break;

	case OW_HANDLER_STATE_READ_BIT_S2:
		if (ow_timing)
			break;

		read_tmp = OW_PIN_HLVL;

		CSECTION_UNLOCK;

		OW_DELAY60;
		handler_state = OW_HANDLER_STATE_READ_BIT_S3;
		break;

	case OW_HANDLER_STATE_READ_BIT_S3:
		if (ow_timing)
			break;

		handler_state = handler_state_return;
		break;

/*
 * WRITE BIT
 *

	case OW_HANDLER_STATE_WRITE_BIT:
		CSECTION_LOCK(CSECTION_OW);

		OW_PIN_LO;
		OW_PIN_OUT;

		if (write_tmp & 0x01)
			OW_DELAY10;
		else
			OW_DELAY70;

		handler_state = OW_HANDLER_STATE_WRITE_BIT_S1;
		break;

	case OW_HANDLER_STATE_WRITE_BIT_S1:
		if (ow_timing)
			break;

		CSECTION_UNLOCK;

		OW_PIN_IN;

		if (write_tmp & 0x01)
			OW_DELAY60;
		else
			OW_DELAY10;

		handler_state = OW_HANDLER_STATE_WRITE_BIT_S2;
		break;

	case OW_HANDLER_STATE_WRITE_BIT_S2:
		if (ow_timing)
			break;

		handler_state = handler_state_return;
		break;

/*
 * READ
 *

	case OW_HANDLER_STATE_READ:
		read_tmp = 0;
		t1 = 8;
		t2 = 0;
		handler_state = OW_HANDLER_STATE_READ_BIT;
		handler_state_return = OW_HANDLER_STATE_READ_S1;
		break;

	case OW_HANDLER_STATE_READ_S1:
		t2 >>= 1;

		if (read_tmp)
			t2 |= 0x80;

		t1--;
		read_tmp = 0;

		if (t1 == 0) {
			read_tmp = t2;
			handler_state = OW_HANDLER_STATE_DONE;
			break;
		}

		handler_state = OW_HANDLER_STATE_READ_BIT;
		handler_state_return = OW_HANDLER_STATE_READ_S1;
		break;

/*
 * WRITE
 *

	case OW_HANDLER_STATE_WRITE:
		t1 = 8;
		handler_state = OW_HANDLER_STATE_WRITE_BIT;
		handler_state_return = OW_HANDLER_STATE_WRITE_S1;
		break;

	case OW_HANDLER_STATE_WRITE_S1:
		write_tmp >>= 1;
		t1--;

		if (t1 == 0) {
			handler_state = OW_HANDLER_STATE_DONE;
			break;
		}

		handler_state = OW_HANDLER_STATE_WRITE_BIT;
		handler_state_return = OW_HANDLER_STATE_WRITE_S1;
		break;

	case OW_HANDLER_STATE_DONE:
		break;

	default:
		break;
	}

	if (search_state != OW_SEARCH_STATE_DONE)
		ow_search_handler();
}

/*
 * SEARCH Algorithm
 *

uint8_t ow_search_start()
{
	if (!ow_search_done())
		return OW_SEARCH_FALSE;

	search_state = OW_SEARCH_STATE_START;
	search_state_return = OW_SEARCH_STATE_START;

	return OW_SEARCH_TRUE;
}

uint8_t ow_search_next()
{
	if (!ow_search_done())
		return OW_SEARCH_FALSE;

	search_state = OW_SEARCH_STATE_NEXT;
	search_state_return = OW_SEARCH_STATE_NEXT;

	return OW_SEARCH_TRUE;
}

static void ow_search_handler()
{
	static uint8_t id_bit_number;
	static uint8_t last_zero;
	static uint8_t rom_byte_number;
	static uint8_t id_bit;
	static uint8_t cmp_id_bit;
	static uint8_t rom_byte_mask;
	static uint8_t search_direction;

	static uint8_t crc8;

	switch (search_state) {

	case OW_SEARCH_STATE_START:
		id_bit_number 			= 1;
		last_zero 				= 0;
		rom_byte_number 		= 0;
		id_bit 					= 0;
		cmp_id_bit 				= 0;
		rom_byte_mask 			= 1;
		search_direction 		= 0;

		LastDiscrepancy 		= 0;
		LastDeviceFlag 			= 0;
		LastFamilyDiscrepancy 	= 0;

		search_status 			= OW_SEARCH_DONE_NONE;

		memset((void*)rom_tmp, 0, sizeof(rom_tmp));

		ow_reset();
		search_state = OW_SEARCH_STATE_RRW_DONE;
		search_state_return = OW_SEARCH_STATE_SEND_SEARCH;
		break;

	case OW_SEARCH_STATE_NEXT:
		id_bit_number 			= 1;
		last_zero 				= 0;
		rom_byte_number 		= 0;
		id_bit 					= 0;
		cmp_id_bit 				= 0;
		rom_byte_mask 			= 1;
		search_direction 		= 0;

		search_status 			= OW_SEARCH_DONE_NONE;

		if (LastDeviceFlag) {
			search_state = OW_SEARCH_STATE_DONE;
			break;
		}

		ow_reset();
		search_state = OW_SEARCH_STATE_RRW_DONE;
		search_state_return = OW_SEARCH_STATE_SEND_SEARCH;
		break;

	case OW_SEARCH_STATE_SEND_SEARCH:
		if (handler_status != OW_HANDLER_STATUS_DONE) {
			search_state = OW_SEARCH_STATE_DONE;
			break;
		}

		write_tmp = OW_SEARCH_ROM;

		ow_write();
		search_state = OW_SEARCH_STATE_RRW_DONE;
		search_state_return = OW_SEARCH_STATE_SEND_READ_ID_BIT;
		break;

	case OW_SEARCH_STATE_SEND_READ_ID_BIT:
		ow_read_bit();
		search_state = OW_SEARCH_STATE_RRW_DONE;
		search_state_return = OW_SEARCH_STATE_SEND_READ_CMP_ID_BIT;
		break;

	case OW_SEARCH_STATE_SEND_READ_CMP_ID_BIT:

		id_bit = read_tmp;

		ow_read_bit();
		search_state = OW_SEARCH_STATE_RRW_DONE;
		search_state_return = OW_SEARCH_STATE_D;
		break;

	case OW_SEARCH_STATE_D:

		cmp_id_bit = read_tmp;

		if ((id_bit == 1) && (cmp_id_bit == 1)) {
			search_state = OW_SEARCH_STATE_DONE;
			search_status = OW_SEARCH_DONE_NONE;
			break;
		}

		if (id_bit != cmp_id_bit) {
			search_direction = id_bit;
		} else {
			if (id_bit_number < LastDiscrepancy)
				search_direction = ((rom_tmp[rom_byte_number] & rom_byte_mask) > 0);
			else
				search_direction = (id_bit_number == LastDiscrepancy);

			if (search_direction == 0) {
				last_zero = id_bit_number;
				if (last_zero < 9)
					LastFamilyDiscrepancy = last_zero;
			}
		}

		if (search_direction == 1)
			rom_tmp[rom_byte_number] |= rom_byte_mask;
		else
			rom_tmp[rom_byte_number] &= ~rom_byte_mask;

		write_tmp = search_direction;

		ow_write_bit();
		search_state = OW_SEARCH_STATE_RRW_DONE;
		search_state_return = OW_SEARCH_STATE_D2;
		break;

	case OW_SEARCH_STATE_D2:

		id_bit_number++;
		rom_byte_mask <<= 1;

		if (rom_byte_mask == 0) {
			crc8 = crc8_ow2(crc8, rom_tmp[rom_byte_number]);
			rom_byte_number++;
			rom_byte_mask = 1;
		}

		if (rom_byte_number < 8)
			search_state = OW_SEARCH_STATE_SEND_READ_ID_BIT;
		else
			search_state = OW_SEARCH_STATE_D3;

		break;

	case OW_SEARCH_STATE_D3:

		if (!((id_bit_number < 65) || (crc8 != 0))) {
			LastDiscrepancy = last_zero;

			if (LastDiscrepancy == 0) {
				LastDeviceFlag = 1;
				search_status = OW_SEARCH_DONE_LAST;
			} else {
				search_status = OW_SEARCH_DONE;
			}
		}

		search_state = OW_SEARCH_STATE_DONE;

		break;

	//////

	case OW_SEARCH_STATE_DONE:
		break;

	//////


	case OW_SEARCH_STATE_RRW_DONE:
		if (ow_done())
			search_state = search_state_return;
		break;

	default:
		break;
	}
}

uint8_t ow_search_done()
{
	return search_state == OW_SEARCH_STATE_DONE;
}

uint8_t *ow_search_result()
{
	if (search_status)
		return rom_tmp;

	return NULL;
}

/**/

enum OwOperation {
  owopDone,
  owopReset,
  owopReadBit,
  owopRead2Bit,
  owopReadByte,
  owopWriteBit,
  owopWriteByte
};

namespace OWire {

  // Timer

  static enum OwOperation CurrentOperation = owopDone;
  static uint8_t CurrentOperationPhase = 0;
  static uint8_t CurrentOperationValue = 0;

  // global declarations for use in class and timer callback
  static GPIO_TypeDef *txPort;
  static uint8_t txPin;
  static GPIO_TypeDef *rxPort;
  static uint8_t rxPin;

  void TimerSetIntervalMks(uint32_t timemks) {

    return;
  }

  void TimerHandler(GPTDriver* gpt) {
    OWDriver& owd = *static_cast<OWDriver*>(gpt->customData);

    gptStopTimerI(gpt);

    switch (CurrentOperation) {
      case owopDone:
        break;

      case owopReset:
        switch (CurrentOperationPhase) {
          case 0:
            palSetPad(txPort, txPin);
            CurrentOperationValue = 0;
            CurrentOperationPhase++;
            gptStartOneShotI(gpt, 150);
            break;
          case 1:
            CurrentOperationValue = palReadPad(rxPort, rxPin);
            CurrentOperationPhase++;
            gptStartOneShotI(gpt, 200);
          break;
          case 2:
            CurrentOperation = owopDone;
          break;
        }
      break;

      case owopReadBit:
        switch (CurrentOperationPhase) {
          case 0:
            palSetPad(txPort, txPin);
            CurrentOperationValue = 0;
            CurrentOperationPhase++;
            gptStartOneShotI(gpt, 10);
            break;
          case 1:
            CurrentOperationValue = palReadPad(rxPort, rxPin);
            CurrentOperationPhase++;
            gptStartOneShotI(gpt, 10);
          break;
          case 2:
            CurrentOperation = owopDone;
          break;
        }
      break;

      case owopWriteBit:
        switch (CurrentOperationPhase) {
          case 0:
            if (CurrentOperationValue)
              palSetPad(txPort, txPin);
            CurrentOperationPhase++;
            gptStartOneShotI(gpt, 45);
            break;
          case 1:
            palSetPad(txPort, txPin);
            CurrentOperationPhase++;
            gptStartOneShotI(gpt, 10);
          break;
          case 2:
            CurrentOperation = owopDone;
          break;
        }
      break;

      default:
        CurrentOperation = owopDone;
      break;
    }
    return;
  }

  const GPTConfig gptconf_{1000000, TimerHandler, 0, 0};


  // OWDriver

  bool OWDriver::Reset(bool *networkHaveDevice) {
    *networkHaveDevice = false;
    if (palReadPad(rxPort, rxPin) != PAL_HIGH)
      return false;

    palClearPad(txPort, txPin);
    CurrentOperation = owopReset;
    CurrentOperationPhase = 0;
    TimerSetIntervalMks(5);

    sleep(MS2ST(1)); //ms

    if (CurrentOperation != owopDone){
      TimerDisable();
      CurrentOperation = owopDone;
      return false;
    }

    *networkHaveDevice = (CurrentOperationValue == 0);

    return true;
  }

  bool OWDriver::ReadBit(bool *bit) {
    *bit = false;
    if (palReadPad(rxPort, rxPin) != PAL_HIGH)
      return false;

    palClearPad(txPort, txPin);
    CurrentOperation = owopReadBit;
    CurrentOperationPhase = 0;
    TimerSetIntervalMks(5);

    return true;
  }

  bool OWDriver::Read2Bit(uint8_t *bit) {
    *bit = 0x0;

    return true;
  }

  bool OWDriver::WriteBit(bool bit) {
    if (palReadPad(rxPort, rxPin) != PAL_HIGH)
      return false;

    palClearPad(txPort, txPin);
    CurrentOperation = owopWriteBit;
    CurrentOperationPhase = 0;
    CurrentOperationValue = bit;
    TimerSetIntervalMks(5);

    return true;
  }

  bool OWDriver::ReadByte(uint8_t *bit) {
    *bit = 0;

    return true;
  }

  bool OWDriver::WriteByte(uint8_t bit) {

    return true;
  }

  void OWDriver::Init(GPIO_TypeDef *_txPort, uint8_t _txPin, GPIO_TypeDef *_rxPort, uint8_t _rxPin) {
    txPort = _txPort;
    txPin  = _txPin;
    rxPort = _rxPort;
    rxPin  = _rxPin;

    GPTD_->customData = static_cast<OWDriver*>(this);
    gptStart(GPTD_, &gptconf_);

    palSetPadMode(txPort, txPin, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(rxPort, rxPin, PAL_MODE_INPUT);
    palSetPad(txPort, txPin);
  }

}
