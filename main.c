/*
  main.c - An embedded CNC Controller with rs274/ngc (g-code) support
  Part of LasaurGrbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud

  LasaurGrbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  LasaurGrbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#include <stdbool.h>
#include <stdint.h>

#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>

#include <driverlib/gpio.h>
#include "driverlib/rom.h"
#include <driverlib/sysctl.h>

#include "config.h"

#include "planner.h"
#include "stepper.h"
#include "sense_control.h"
#include "temperature.h"
#include "gcode.h"
#include "serial.h"
#include "joystick.h"
#include "tasks.h"
#include "lcd.h"

#if defined(PART_TM4C123GH6PM)
#include "inc/tm4c123gh6pm.h"
#elif defined(PART_TM4C1233H6PM)
#include "inc/lm4f120h5qr.h"
#else
#error "Please set a valid PART_ define"
#endif

/* Main */
int main(void)
{
    //
    // Enable lazy stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    ROM_FPULazyStackingEnable();

    // Configure system clock
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN); //80MHZ

    /* Initialise the GPIO Peripherals */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    /* Setup the LED GPIO pins used */
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1); /* LM4F120H5QR_LED_RED */
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2); /* LM4F120H5QR_LED_BLUE */
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3); /* LM4F120H5QR_LED_GREEN */

    /* Setup the button GPIO pins used */
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);  /* LM4F120H5QR_GPIO_SW1 */
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);

    /* PF0 requires unlocking before configuration */
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= GPIO_PIN_0;
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);  /* LM4F120H5QR_GPIO_SW2 */
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_M;

    /* Turn off user LEDs */
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1, 0);

    /* Initialize GRBL */
    tasks_init();

    joystick_init();

    // This needs to be done before the USB interrupts start firing
    temperature_init();
#ifdef ENABLE_LCD
    lcd_init();
#endif
    serial_init();
    gcode_init();
    sense_init();
    control_init();
    planner_init();
    stepper_init();

    while (true) {
    	tasks_loop();
    }

    return (0);
}

