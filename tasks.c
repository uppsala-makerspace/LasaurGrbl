/*
  tasks.c - manage non-ISR tasks
  Part of LasaurGrbl

  Copyright (c) 2013 Richard Taylor

  LasaurGrbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  LasaurGrbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/
#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>

#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"

#include "tasks.h"

#include "gcode.h"
#include "planner.h"
#include "serial.h"
#include "stepper.h"


static volatile task_t task_status = 0;
static void *task_data[sizeof(task_t)];

void tasks_init(void) {
	task_status = 0;
}

void task_enable(TASK task, void* data) {
	task_status |= (1 << task);
	task_data[task] = data;
}

void task_disable(TASK task) {
	task_status &= ~(1 << task);
	task_data[task] = 0;
}

uint8_t task_running(TASK task) {
	if (task_status & (1 << task))
		return 1;
	return 0;
}

void tasks_loop(void) {

	// Main task loop, does not return
	// None of the tasks should block, other than
	// when performing work.
	while (1) {

		// Wait for the machine to be ready (available blocks)
    	if (task_running(TASK_READY_WAIT)) {
    		if (planner_blocks_available() >= 2)
			{
				char tmp[2] = {0x12, 0};
				printString(tmp);
				task_disable(TASK_READY_WAIT);
			}
    	}

		// Process any serial data available
    	if (task_running(TASK_SERIAL_RX)) {
    		if (gcode_process_data(task_data[TASK_SERIAL_RX]) == 0) {
    			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
    			task_disable(TASK_SERIAL_RX);
    		}
    	}

		// Process manual moves
    	if (task_running(TASK_MANUAL_MOVE)) {
    		struct task_manual_move_data *move = task_data[TASK_MANUAL_MOVE];
    		if (planner_blocks_available() >= 2) {
    			gcode_manual_move(move->x_offset, move->y_offset, move->rate);
    			task_disable(TASK_MANUAL_MOVE);
    		}
    	}

		// Process offset set
    	if (task_running(TASK_SET_OFFSET)) {
			if (!stepper_active()) {
				gcode_set_offset_to_current_position();
			}
			task_disable(TASK_SET_OFFSET);
    	}
	}
}

