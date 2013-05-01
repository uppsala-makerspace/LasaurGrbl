/*
  config.h - compile time configuration
  Part of LasaurGrbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud
  Copyright (c) 2011 Sungeun K. Jeon
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

#ifndef config_h
#define config_h

#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_timer.h>
#include <inc/hw_ints.h>
#include <inc/hw_gpio.h>

#include <driverlib/gpio.h>
#include "driverlib/rom.h"
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Version number
// (must not contain capital letters)
#define LASAURGRBL_VERSION "13.04"
//#define DEBUG_IGNORE_SENSORS  // set for debugging

#define CONFIG_X_STEPS_PER_MM 32.80839895 //microsteps/mm
#define CONFIG_Y_STEPS_PER_MM 32.80839895 //microsteps/mm
#define CONFIG_Z_STEPS_PER_MM 32.80839895 //microsteps/mm
#define CONFIG_PULSE_MICROSECONDS 5
#define CONFIG_FEEDRATE 8000.0 // in millimeters per minute
#define CONFIG_SEEKRATE 8000.0
#define CONFIG_ACCELERATION 1200000.0 // mm/min^2, typically 1000000-8000000, divide by (60*60) to get mm/sec^2
#define CONFIG_JUNCTION_DEVIATION 0.006 // mm
#define CONFIG_X_ORIGIN_OFFSET 5.0  // mm, x-offset of table origin from physical home
#define CONFIG_Y_ORIGIN_OFFSET 5.0  // mm, y-offset of table origin from physical home
#define CONFIG_Z_ORIGIN_OFFSET 0.0   // mm, z-offset of table origin from physical home
#define CONFIG_INVERT_X_AXIS 1  // 0 is regular, 1 inverts the x direction
#define CONFIG_INVERT_Y_AXIS 1  // 0 is regular, 1 inverts the y direction
#define CONFIG_INVERT_Z_AXIS 1  // 0 is regular, 1 inverts the y direction

#define SENSE_PORT              GPIO_PORTE_BASE
#define CHILLER_BIT             0
#define DOOR_BIT                1
#define SENSE_MASK 				((1<<CHILLER_BIT)|(1<<DOOR_BIT))

#define SENSE_TIMER				TIMER2_BASE

#define OW_PORT					GPIO_PORTE_BASE
#define OW_BIT             		5

#define ASSIST_PORT           	GPIO_PORTD_BASE
#define AIR_ASSIST_BIT        	2
#define AUX1_ASSIST_BIT       	3
#define AUX2_ASSIST_BIT       	6
#define ASSIST_MASK				(1 << AIR_ASSIST_BIT) | (1<< AUX1_ASSIST_BIT) | (1<< AUX2_ASSIST_BIT)
  
#define LIMIT_PORT              GPIO_PORTC_BASE
#define X_LIMIT_BIT             4
#define Y_LIMIT_BIT             5
#define Z_LIMIT_BIT             6
#define E_LIMIT_BIT             7
#define LIMIT_MASK 				((1<<X_LIMIT_BIT)|(1<<Y_LIMIT_BIT)|(1<<Z_LIMIT_BIT)|(1<<E_LIMIT_BIT))


#define STEPPING_TIMER			TIMER1_BASE

#define STEP_EN_PORT         	GPIO_PORTB_BASE
#define STEP_X_EN           	0
#define STEP_Y_EN           	1
#define STEP_Z_EN           	2
#define STEP_EN_MASK			((1 << STEP_X_EN) | (1 << STEP_Y_EN) | (1 << STEP_Z_EN))

#define STEP_DIR_PORT         	GPIO_PORTB_BASE
#define STEP_X_DIR           	3
#define STEP_Y_DIR           	4
#define STEP_Z_DIR           	5
#define STEP_DIR_MASK			((1 << STEP_X_DIR) | (1 << STEP_Y_DIR) | (1 << STEP_Z_DIR))
#define STEP_DIR_INVERT			((CONFIG_INVERT_X_AXIS<<STEP_X_DIR)|(CONFIG_INVERT_Y_AXIS<<STEP_Y_DIR)|(CONFIG_INVERT_Z_AXIS<<STEP_Z_DIR))

#define STEP_PORT           	GPIO_PORTE_BASE
#define STEP_X_BIT              2
#define STEP_Y_BIT              3
#define STEP_Z_BIT              4
#define STEP_MASK				((1 << STEP_X_BIT) | (1 << STEP_Y_BIT) | (1 << STEP_Z_BIT))

#define LASER_PORT           	GPIO_PORTB_BASE
#define LASER_BIT        		6
#define LASER_TIMER				TIMER0_BASE
#define LASER_PWM_FREQ			40000

// The temporal resolution of the acceleration management subsystem. Higher number give smoother
// acceleration but may impact performance.
// NOTE: Increasing this parameter will help any resolution related issues, especially with machines 
// requiring very high accelerations and/or very fast feedrates. In general, this will reduce the 
// error between how the planner plans the motions and how the stepper program actually performs them.
// However, at some point, the resolution can be high enough, where the errors related to numerical 
// round-off can be great enough to cause problems and/or it's too fast for the Arduino. The correct
// value for this parameter is machine dependent, so it's advised to set this only as high as needed.
// Approximate successful values can range from 30L to 100L or more.
#define ACCELERATION_TICKS_PER_SECOND 200L

// Minimum planner junction speed. Sets the default minimum speed the planner plans for at the end
// of the buffer and all stops. This should not be much greater than zero and should only be changed
// if unwanted behavior is observed on a user's machine when running at very slow speeds.
#define ZERO_SPEED 0.0 // (mm/min)

// Minimum stepper rate. Sets the absolute minimum stepper rate in the stepper program and never runs
// slower than this value, except when sleeping. This parameter overrides the minimum planner speed.
// This is primarily used to guarantee that the end of a movement is always reached and not stop to
// never reach its target. This parameter should always be greater than zero.
#define MINIMUM_STEPS_PER_MINUTE 1600U // (steps/min) - Integer value only
// 1600 @ 32step_per_mm = 50mm/min
  

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2


#define clear_vector(a) memset(a, 0, sizeof(a))
#define clear_vector_double(a) memset(a, 0.0, sizeof(a))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))


#endif



// bit math
// see: http://www.arduino.cc/playground/Code/BitMath
// see: http://graphics.stanford.edu/~seander/bithacks.html
//
// y = (x >> n) & 1; // n=0..15. stores nth bit of x in y. y becomes 0 or 1.
//
// x &= ~(1 << n); // forces nth bit of x to be 0. all other bits left alone.
//
// x &= (1<<(n+1))-1; // leaves alone the lowest n bits of x; all higher bits set to 0.
//
// x |= (1 << n); // forces nth bit of x to be 1. all other bits left alone.
//
// x ^= (1 << n); // toggles nth bit of x. all other bits left alone.
//
// x = ~x; // toggles ALL the bits in x.

void __delay_us(uint32_t delay);
