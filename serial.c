/*
  serial.c - Low level functions for sending and recieving bytes via the serial port.
  Part of LasaurGrbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud
  Copyright (c) 2011 Sungeun K. Jeon
  Copyright (c) 2011 Stefan Hechenberger

  Inspired by the wiring_serial module by David A. Mellis which
  used to be a part of the Arduino project.
   
  LasaurGrbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  LasaurGrbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#include <string.h>

#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_timer.h>
#include <inc/hw_gpio.h>
#include <inc/hw_ints.h>

#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>
#include <driverlib/interrupt.h>

#include "config.h"

#include "USBCDCD.h"

#include "serial.h"
#include "stepper.h"
#include "gcode.h"

void serial_init() {
    //
    // Configure the required pins for USB operation.
    //
    GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_5 | GPIO_PIN_4);

    /* Enable the USB peripheral and PLL */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_USB0);
    SysCtlUSBPLLEnable();

    /* Setup pins for USB operation */
    GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    USBCDCD_init();

    IntPrioritySet(INT_USB0, CONFIG_USB_PRIORITY);
}

void printString(const char *s) {
    USBCDCD_sendData((const uint8_t*)s, strlen(s));
}

// Print a string stored in PGM-memory
void printPgmString(const char *s) {
    USBCDCD_sendData((const uint8_t*)s, strlen(s));
}

void serial_write(uint8_t data) {
    USBCDCD_sendData(&data, 1);
}

void printIntegerInBase(unsigned long n, unsigned long base) {
  unsigned char buf[8 * sizeof(long)]; // Assumes 8-bit chars.
  unsigned long i = 0;

  if (n == 0) {
    serial_write('0');
    return;
  }

  while (n > 0) {
    buf[i++] = n % base;
    n /= base;
  }

  for (; i > 0; i--) {
	serial_write((buf[i - 1] < 10)?'0' + buf[i - 1]:'A' + buf[i - 1] - 10);
  }
}

void printInteger(long n) {
  if (n < 0) {
    serial_write('-');
    n = -n;
  }

  printIntegerInBase(n, 10);
}

void printFloat(double n) {
  if (n < 0) {
    serial_write('-');
    n = -n;
  }
  n += 0.5/1000; // Add rounding factor

  long integer_part;
  integer_part = (int)n;
  printIntegerInBase(integer_part,10);

  serial_write('.');

  n -= integer_part;
  int decimals = 3;
  uint8_t decimal_part;
  while(decimals-- > 0) {
    n *= 10;
    decimal_part = (int) n;
    serial_write('0'+decimal_part);
    n -= decimal_part;
  }
}

