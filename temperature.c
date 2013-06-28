/*
  temperature.c - 1-wire temperature sensor access
  Configures and periodically reads temperature value from up to
  3 DS sensors attached to the 1-wire bus.

  Copyright (c) 2013 Richard Taylor

  LasaurGrbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  LasaurGrbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  ---
*/

#include <string.h>

#include <inc/hw_ints.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_timer.h>
#include <inc/hw_gpio.h>

#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>
#include <driverlib/interrupt.h>

#include "config.h"

#include "temperature.h"

#define OW_DELAY_A 6
#define OW_DELAY_B 64
#define OW_DELAY_C 60
#define OW_DELAY_D 10
#define OW_DELAY_E 7
#define OW_DELAY_F 55
#define OW_DELAY_G 1
#define OW_DELAY_H 480
#define OW_DELAY_I 70
#define OW_DELAY_J 410

// Sensor ROM Codes
static uint8_t sensor_rom[3][8];
static uint8_t scratch_pad[9] = {0};
static uint8_t num_sensors = 0;
static uint16_t temperature[3] = {0};
static uint64_t timer_load;

static void ow_write_byte(uint8_t data);
static uint8_t ow_search_first(uint8_t *rom);
static uint8_t ow_search_next(uint8_t *rom);

static uint32_t cycles_per_us = 0;
static uint32_t timer_calibration = 0;

void timer_cal_isr(void) {
	TimerLoadSet64(SENSE_TIMER, timer_load);
	TimerIntClear(SENSE_TIMER, TIMER_TIMA_TIMEOUT);
	timer_calibration++;
}

void __delay_us(uint32_t delay) {
	if (delay == 0) return;
    SysCtlDelay(cycles_per_us * delay);
}

#define OW_SET_HIGH		0x8000
#define OW_SET_LOW		0x4000
#define OW_SAMPLE		0xC000

typedef enum
{
	OW_BYTE_RESET,
	OW_BYTE_READ,
	OW_BYTE_WRITE
} OW_BYTE_STATE;

static const uint16_t ow_reset_timings[] = {OW_DELAY_G,
									  OW_SET_LOW | OW_DELAY_H,
									  OW_SET_HIGH | OW_DELAY_I,
		                              OW_SAMPLE | OW_DELAY_J,
		                              0};

static const uint16_t ow_read_timings[] = {OW_SET_LOW | OW_DELAY_A,
		                             OW_SET_HIGH | OW_DELAY_E,
		                             OW_SAMPLE | OW_DELAY_F,
		                             0};

static const uint16_t ow_write0_timings[] = {OW_SET_LOW | OW_DELAY_C,
		                               OW_SET_HIGH | OW_DELAY_D,
		                               OW_SAMPLE,
		                               0};

static const uint16_t ow_write1_timings[] = {OW_SET_LOW | OW_DELAY_A,
		                               OW_SET_HIGH | OW_DELAY_B,
		                               OW_SAMPLE,
		                               0};

static const uint16_t *ow_bit_state = ow_reset_timings;
static uint8_t ow_bit_index = 0;
static uint8_t ow_bit = 0;

static uint8_t ow_bit_count = 0;
static uint8_t ow_byte = 0;
static OW_BYTE_STATE ow_byte_state = OW_BYTE_RESET;


static uint8_t cur_rom = 0;
static uint8_t cur_rom_byte = 0;
static uint8_t cur_scratch_byte = 0;
static uint8_t cur_state = 0;

void temperature_update_isr(void) {
	uint16_t bit_action = 0;
	uint16_t bit_delay = 0;

	if (ow_bit_state != 0)
		bit_action = ow_bit_state[ow_bit_index++];

	bit_delay = bit_action & 0x0FFF;
	bit_action &= 0xF000;

	// One Wire Bit-State Machine
	switch (bit_action)
	{
		case OW_SET_HIGH:
			GPIOPinWrite(OW_PORT, (1 << OW_BIT), (1 << OW_BIT)); // Releases the bus
			GPIOPinTypeGPIOInput(OW_PORT, (1 << OW_BIT));
		break;

		case OW_SET_LOW:
			GPIOPinTypeGPIOOutput(OW_PORT, (1 << OW_BIT));
			GPIOPinWrite(OW_PORT, (1 << OW_BIT), 0x00); // Drives DQ low
		break;

		case OW_SAMPLE:
			// Sample bit.
			ow_bit = (GPIOPinRead(OW_PORT, (1 << OW_BIT)) != 0)?1:0;
		break;

		default:
			// End of bit sequence
		break;
	}

	// One Wire Byte State Machine
	if (bit_delay == 0)
	{
		// Make sure we are scheduled again.
		bit_delay = 1;
		ow_bit_index = 0;

		switch(ow_byte_state)
		{
			case OW_BYTE_RESET:
				ow_bit_state = 0;
			break;

			case OW_BYTE_READ:
				if (ow_bit != 0)
					ow_byte |= 0x80; //(1 << ow_bit_count);
				ow_bit_count++;
				if (ow_bit_count > 7)
					ow_bit_state = 0;
				else
					ow_byte >>= 1;
			break;

			case OW_BYTE_WRITE:
				if ((ow_byte & 0x01) != 0)
					ow_bit_state = ow_write1_timings;
				else
					ow_bit_state = ow_write0_timings;
				ow_byte >>= 1;
				ow_bit_count++;
				if (ow_bit_count > 8)
					ow_bit_state = 0;
			break;
		default:
			ow_bit_state = 0;
			break;
		}
	}

	// Temperature State Machine
	if (ow_bit_state == 0)
	{
		// Reset the bit machine
		ow_bit_count = 0;

		// Default to fire a timer in 1us.
		bit_delay = 1;

		switch (cur_state)
		{
			case 0:
				// Send a reset
				ow_byte_state = OW_BYTE_RESET;
				ow_bit_state = ow_reset_timings;
				if (cur_rom >= num_sensors)
				{
					// Start a new conversion
					cur_rom = 0;
					cur_state = 7;
				}
				else
				{
					// Read our device(s)
					cur_state++;
				}
			break;

			case 1:
				if (ow_bit != 0)
				{
					cur_state = 0;
					bit_delay = 0;
					break;
				}

				// Send the Match ROM Command
				ow_byte_state = OW_BYTE_WRITE;
				ow_byte = 0x55;
				cur_state++;
			break;

			case 2:
				// Send the ROM bytes
				ow_byte_state = OW_BYTE_WRITE;
				ow_byte = sensor_rom[cur_rom][cur_rom_byte++];
				if (cur_rom_byte > 7)
				{
					cur_rom_byte = 0;
					cur_state++;
				}
			break;

			case 3:
				// Send the Read Scratchpad Command
				ow_byte_state = OW_BYTE_WRITE;
				ow_byte = 0xBE;
				cur_state++;
			break;

			case 4:
				// Read a byte
				ow_bit_state = ow_read_timings;
				ow_byte_state = OW_BYTE_READ;
				ow_byte = 0x00;
				cur_state++;
			break;

			case 5:
				scratch_pad[cur_scratch_byte++] = ow_byte;

				if (cur_scratch_byte < 9)
				{
					// Read a byte
					ow_bit_state = ow_read_timings;
					ow_byte_state = OW_BYTE_READ;
					ow_byte = 0x00;
					break;
				}

				if (scratch_pad[5] == 0xFF && scratch_pad[7] == 0x10 && scratch_pad[1] < 7)
				{
					temperature[cur_rom] = scratch_pad[0] | (scratch_pad[1] << 8);
				}

				cur_scratch_byte = 0;
				cur_rom++;
				cur_state = 0;
			break;

			case 6:
			break;

			case 7:
				// Send the Skip ROM Command
				ow_byte_state = OW_BYTE_WRITE;
				ow_byte = 0xCC;
				cur_state++;
			break;

			case 8:
				// Send the Start Conversion command
				ow_byte_state = OW_BYTE_WRITE;
				ow_byte = 0x44;
				cur_state++;
			break;

			case 9:
				// Schedule an update in ~5s.
				bit_delay = 0;
				cur_state = 0;
			break;

			default:
			break;
		}
	}

	// Schedule timer to go off
	if (bit_delay > 0)
	{
		TimerLoadSet64(SENSE_TIMER, (SysCtlClockGet() / 1000000) * bit_delay);
	}
	else
	{
		TimerLoadSet64(SENSE_TIMER, timer_load);
	}

	TimerIntClear(SENSE_TIMER, TIMER_TIMA_TIMEOUT);
}

void temperature_init(void) {
	int rom;

	// Configure timer
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
	TimerConfigure(SENSE_TIMER, TIMER_CFG_PERIODIC);

	// Create a 1us timer
	timer_load = SysCtlClockGet() / 1000;
	TimerLoadSet64(SENSE_TIMER, timer_load);

	TimerIntRegister(SENSE_TIMER, TIMER_A, timer_cal_isr);
	TimerIntEnable(SENSE_TIMER, TIMER_TIMA_TIMEOUT);
	IntPrioritySet(INT_TIMER2A, CONFIG_SENSE_PRIORITY);


	cycles_per_us = 1;
	timer_calibration = 0;
	TimerEnable(SENSE_TIMER, TIMER_A);
	__delay_us(SysCtlClockGet() / 1000);
	cycles_per_us = SysCtlClockGet() / timer_calibration / 1000000;
	TimerDisable(SENSE_TIMER, TIMER_A);

	// 1-Wire Port
	GPIOPinWrite(OW_PORT, (1 << OW_BIT), (1 << OW_BIT)); // Releases the bus
	GPIOPinTypeGPIOOutput(OW_PORT, (1 << OW_BIT));
	GPIOPadConfigSet(OW_PORT, (1 << OW_BIT), GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_OD_WPU);

	// We Support up to 3 sensors
	num_sensors = 0;
	rom = ow_search_first(sensor_rom[0]);
	if (rom)
		num_sensors++;
	while (rom && num_sensors < 3)
		rom = ow_search_next(sensor_rom[num_sensors++]);

	// Create a timer ISR to fire the conversions read the temperature.
	// The minimum conversion time is approx. 750ms.
	timer_load = SysCtlClockGet() * 1;
	TimerLoadSet64(SENSE_TIMER, timer_load);

	// Don't bother starting the timer if no sensors were found
	if (num_sensors > 0)
	{
		TimerIntRegister(SENSE_TIMER, TIMER_A, temperature_update_isr);
		TimerEnable(SENSE_TIMER, TIMER_A);
	}
}

// Returns a 12-bit right-justified 2's complement in 1/16ths of a Degree C.
uint16_t temperature_read(uint8_t sensor)
{
	return temperature[sensor];
}

uint8_t temperature_num_sensors(void)
{
	return num_sensors;
}

//-----------------------------------------------------------------------------
// Generate a 1-Wire reset, return 1 if no presence detect was found,
// return 0 otherwise.
// (NOTE: Does not handle alarm presence from DS2404/DS1994)
//
static int ow_reset(void)
{
	uint8_t result;

	GPIOPinTypeGPIOOutput(OW_PORT, (1 << OW_BIT));
	__delay_us(OW_DELAY_G);
	GPIOPinWrite(OW_PORT, (1 << OW_BIT), 0x00); // Drives DQ low
	__delay_us(OW_DELAY_H);
	GPIOPinWrite(OW_PORT, (1 << OW_BIT), (1 << OW_BIT)); // Releases the bus
	GPIOPinTypeGPIOInput(OW_PORT, (1 << OW_BIT));
	__delay_us(OW_DELAY_I);
	result = GPIOPinRead(OW_PORT, (1 << OW_BIT)); // Sample for presence pulse from slave
	__delay_us(OW_DELAY_J); // Complete the reset sequence recovery
    return (result!=0)?0:1;
}

//-----------------------------------------------------------------------------
// Send a 1-Wire write bit. Provide 10us recovery time.
//
static void ow_write_bit(uint8_t bit)
{
	GPIOPinTypeGPIOOutput(OW_PORT, (1 << OW_BIT));
	if (bit)
	{
		// Write '1' bit
		GPIOPinWrite(OW_PORT, (1 << OW_BIT), 0x00); // Drives DQ low
		__delay_us(OW_DELAY_A);
		GPIOPinWrite(OW_PORT, (1 << OW_BIT), (1 << OW_BIT)); // Releases the bus
		__delay_us(OW_DELAY_B); // Complete the time slot and 10us recovery
	}
	else
	{
		// Write '0' bit
		GPIOPinWrite(OW_PORT, (1 << OW_BIT), 0x00); // Drives DQ low
		__delay_us(OW_DELAY_C);
		GPIOPinWrite(OW_PORT, (1 << OW_BIT), (1 << OW_BIT)); // Releases the bus
		__delay_us(OW_DELAY_D);
	}
	GPIOPinTypeGPIOInput(OW_PORT, (1 << OW_BIT));
}

//-----------------------------------------------------------------------------
// Read a bit from the 1-Wire bus and return it. Provide 10us recovery time.
//
static int ow_read_bit(void)
{
	uint8_t result;

	GPIOPinTypeGPIOOutput(OW_PORT, (1 << OW_BIT));
	GPIOPinWrite(OW_PORT, (1 << OW_BIT), 0x00); // Drives DQ low
	__delay_us(OW_DELAY_A);
	GPIOPinWrite(OW_PORT, (1 << OW_BIT), (1 << OW_BIT)); // Releases the bus
	GPIOPinTypeGPIOInput(OW_PORT, (1 << OW_BIT));
	__delay_us(OW_DELAY_E);
	result = GPIOPinRead(OW_PORT, (1 << OW_BIT)); // Sample the bit value from the slave
	__delay_us(OW_DELAY_F); // Complete the time slot and 10us recovery

	return (result!=0)?1:0;
}

//-----------------------------------------------------------------------------
// Write 1-Wire data byte
//
static void ow_write_byte(uint8_t data)
{
	uint8_t loop;

	// Loop to write each bit in the byte, LS-bit first
	for (loop = 0; loop < 8; loop++)
	{
		ow_write_bit(data & 0x01);

		// shift the data byte for the next bit
		data >>= 1;
	}
}

static uint8_t LastDiscrepancy;
static uint8_t LastFamilyDiscrepancy;
static uint8_t LastDeviceFlag;
static uint8_t crc8;

static uint8_t dscrc_table[] = {
        0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
      157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
       35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
      190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
       70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
      219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
      101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
      248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
      140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
       17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
      175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
       50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
      202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
       87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
      233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
      116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

//--------------------------------------------------------------------------
// Calculate the CRC8 of the byte value provided with the current
// global 'crc8' value.
// Returns current global crc8 value
//
static uint8_t docrc8(uint8_t value)
{
   // See Application Note 27

   // TEST BUILD
   crc8 = dscrc_table[crc8 ^ value];
   return crc8;
}


static uint8_t ROM_NO[8];
//--------------------------------------------------------------------------
// Perform the 1-Wire Search Algorithm on the 1-Wire bus using the existing
// search state.
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : device not found, end of search
//
static uint8_t ow_search(unsigned char *rom)
{
   uint8_t id_bit_number;
   uint8_t last_zero, rom_byte_number, search_result;
   uint8_t id_bit, cmp_id_bit;
   uint8_t rom_byte_mask, search_direction;

   // initialize for search
   id_bit_number = 1;
   last_zero = 0;
   rom_byte_number = 0;
   rom_byte_mask = 1;
   search_result = 0;
   crc8 = 0;

   // if the last call was not the last one
   if (!LastDeviceFlag)
   {
      // 1-Wire reset
      if (!ow_reset())
      {
         // reset the search
         LastDiscrepancy = 0;
         LastDeviceFlag = 0;
         LastFamilyDiscrepancy = 0;
         return 0;
      }

      // issue the search command
      ow_write_byte(0xF0);

      // loop to do the search
      do
      {
         // read a bit and its complement
         id_bit = ow_read_bit();
         cmp_id_bit = ow_read_bit();

         // check for no devices on 1-wire
         if ((id_bit == 1) && (cmp_id_bit == 1))
            break;
         else
         {
            // all devices coupled have 0 or 1
            if (id_bit != cmp_id_bit)
               search_direction = id_bit;  // bit write value for search
            else
            {
               // if this discrepancy if before the Last Discrepancy
               // on a previous next then pick the same as last time
               if (id_bit_number < LastDiscrepancy)
                  search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
               else
                  // if equal to last pick 1, if not then pick 0
                  search_direction = (id_bit_number == LastDiscrepancy);

               // if 0 was picked then record its position in LastZero
               if (search_direction == 0)
               {
                  last_zero = id_bit_number;

                  // check for Last discrepancy in family
                  if (last_zero < 9)
                     LastFamilyDiscrepancy = last_zero;
               }
            }

            // set or clear the bit in the ROM byte rom_byte_number
            // with mask rom_byte_mask
            if (search_direction == 1)
              ROM_NO[rom_byte_number] |= rom_byte_mask;
            else
              ROM_NO[rom_byte_number] &= ~rom_byte_mask;

            // serial number search direction write bit
            ow_write_bit(search_direction);

            // increment the byte counter id_bit_number
            // and shift the mask rom_byte_mask
            id_bit_number++;
            rom_byte_mask <<= 1;

            // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
            if (rom_byte_mask == 0)
            {
                docrc8(ROM_NO[rom_byte_number]);  // accumulate the CRC
                rom_byte_number++;
                rom_byte_mask = 1;
            }
         }
      }
      while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7

      // if the search was successful then
      if (!((id_bit_number < 65) || (crc8 != 0)))
      {
         // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
         LastDiscrepancy = last_zero;

         // check for last device
         if (LastDiscrepancy == 0)
            LastDeviceFlag = 1;

         search_result = 1;
      }
   }

   // if no device found then reset counters so next 'search' will be like a first
   if (!search_result || !ROM_NO[0])
   {
      LastDiscrepancy = 0;
      LastDeviceFlag = 0;
      LastFamilyDiscrepancy = 0;
      search_result = 0;
   }
   else
	   memcpy(rom, ROM_NO, 8);

   return search_result;
}

//--------------------------------------------------------------------------
// Find the 'first' devices on the 1-Wire bus
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : no device present
//
static uint8_t ow_search_first(unsigned char *rom)
{
   // reset the search state
   LastDiscrepancy = 0;
   LastDeviceFlag = 0;
   LastFamilyDiscrepancy = 0;

   return ow_search(rom);
}

//--------------------------------------------------------------------------
// Find the 'next' devices on the 1-Wire bus
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : device not found, end of search
//
static uint8_t ow_search_next(unsigned char *ROM_NO)
{
   // leave the search state alone
   return ow_search(ROM_NO);
}
