/*
  gcode.c - rs274/ngc parser.
  Part of Grbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef gcode_h
#define gcode_h

#include <inc/hw_types.h>
#include <usblib/usblib.h>

#define STATUS_OK 0
#define STATUS_RX_BUFFER_OVERFLOW 1
#define STATUS_LINE_BUFFER_OVERFLOW 2
#define STATUS_TRANSMISSION_ERROR 3
#define STATUS_BAD_NUMBER_FORMAT 4
#define STATUS_EXPECTED_COMMAND_LETTER 5
#define STATUS_UNSUPPORTED_STATEMENT 6
#define STATUS_SERIAL_STOP_REQUEST 7
#define STATUS_LIMIT_HIT 8
#define STATUS_POWER_OFF 9
// #define STATUS_DOOR_OPEN 10
// #define STATUS_CHILLER_OFF 11

extern uint8_t gcode_ready_wait;

// Initialize the parser
void gcode_init();

// Process a USB buffer and execute line(s) when complete.
void gcode_process_data(const tUSBBuffer *psBuffer);

// process a line of gcode
void gcode_process_line();

// Execute one line of rs275/ngc/g-code
// blocks when 
uint8_t gcode_execute_line(char *line);

// update to stepper position when steppers have been stopped
// called from the stepper code that executes the stop
void gcode_request_position_update();

// Manually moves the machine by these offsets
void gcode_manual_move(double x, double y);

// Set the offsets to the current location
void gcode_set_offset_to_current_position(void);

void gcode_do_home(void);

#endif
