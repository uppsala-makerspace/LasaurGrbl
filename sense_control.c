/*
  sense_control.h - sensing and controlling inputs and outputs
  Part of LasaurGrbl

  Copyright (c) 2011 Stefan Hechenberger

  LasaurGrbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  LasaurGrbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#include <math.h>
#include <stdlib.h>

#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_timer.h>
#include <inc/hw_gpio.h>

#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>

#include "config.h"

#include "sense_control.h"
#include "stepper.h"
#include "planner.h"

static uint32_t laser_cycles;
static uint32_t laser_divider;

void sense_init() {
	//// chiller, door, (power)
	GPIOPinTypeGPIOInput(SENSE_PORT, SENSE_MASK);
	GPIOPadConfigSet(SENSE_PORT, SENSE_MASK, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);

	//// x1_lmit, x2_limit, y1_limit, y2_limit, z1_limit, z2_limit
	GPIOPinTypeGPIOInput(LIMIT_PORT, LIMIT_MASK);
	GPIOPadConfigSet(LIMIT_PORT, LIMIT_MASK, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);
}

void control_init() {
  //// laser control
  // Setup Timer0 for a 488.28125Hz "phase correct PWM" wave (assuming a 16Mhz clock)
  // Timer0 can pwm either PD5 (OC0B) or PD6 (OC0A), we use PD6
  // TCCR0A and TCCR0B are the registers to setup Timer0
  // see chapter "8-bit Timer/Counter0 with PWM" in Atmga328 specs
  // OCR0A sets the duty cycle 0-255 corresponding to 0-100%
  // also see: http://arduino.cc/en/Tutorial/SecretsOfArduinoPWM

	GPIOPinTypeGPIOOutput(LASER_EN_PORT, LASER_EN_MASK);
	GPIOPinWrite(LASER_EN_PORT, LASER_EN_MASK, LASER_EN_INVERT);

	// set refresh rate
	laser_cycles = SysCtlClockGet() / LASER_PWM_FREQ; /*Hz*/
	laser_divider = laser_cycles >> 16;
	laser_cycles /= (laser_divider + 1);

	// Configure timer
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(LASER_TIMER, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM);
	TimerControlLevel(LASER_TIMER, TIMER_BOTH, 1);

	// PPI = PWMfreq/(feedrate/25.4/60)

	// Setup Timer
	TimerPrescaleSet(LASER_TIMER, TIMER_BOTH, laser_divider);
	TimerLoadSet(LASER_TIMER, TIMER_A, laser_cycles);
	TimerPrescaleMatchSet(LASER_TIMER, TIMER_A, laser_divider);

	// Set default value
	control_laser_intensity(0);

	TimerEnable(LASER_TIMER, TIMER_A);

	// ToDo: Map the timer ccp pin sensibly
	GPIOPinConfigure(GPIO_PB6_T0CCP0);
	GPIOPinTypeTimer(LASER_PORT, (1 << LASER_BIT));

	//// air and aux assist control
	GPIOPinTypeGPIOOutput(ASSIST_PORT, ASSIST_MASK);
	control_air_assist(false);
	control_aux1_assist(false);
}


void control_laser_intensity(uint8_t intensity) {
	TimerMatchSet(LASER_TIMER, TIMER_A, laser_cycles - (laser_cycles * intensity / 255));

	if (intensity > 5)
		GPIOPinWrite(LASER_EN_PORT, LASER_EN_MASK, LASER_EN_MASK ^ LASER_EN_INVERT);
	else
		GPIOPinWrite(LASER_EN_PORT, LASER_EN_MASK, LASER_EN_INVERT);
}



void control_air_assist(bool enable) {
  if (enable) {
    GPIOPinWrite(ASSIST_PORT, (1 << AIR_ASSIST_BIT), (1 << AIR_ASSIST_BIT));
  } else {
    GPIOPinWrite(ASSIST_PORT, (1 << AIR_ASSIST_BIT), 0);
  }
}

void control_aux1_assist(bool enable) {
  if (enable) {
    GPIOPinWrite(ASSIST_PORT, (1 << AUX1_ASSIST_BIT), (1 << AUX1_ASSIST_BIT));
  } else {
    GPIOPinWrite(ASSIST_PORT, (1 << AUX1_ASSIST_BIT), 0);
  }  
}
