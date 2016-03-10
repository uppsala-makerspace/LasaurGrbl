/*
  planner.c - buffers movement commands and manages the acceleration profile plan
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


#ifndef planner_h
#define planner_h
                 
#include <stdbool.h>

#include "config.h"

// This needs to be the minimum buffers required for the largest command (raster)
#define PLANNER_FIFO_READY_THRESHOLD		4

// Command types the planner and stepper can schedule for execution
typedef enum {
	BLOCK_TYPE_LINE,
	BLOCK_TYPE_RASTER_LINE,
	BLOCK_TYPE_AIR_ASSIST_ENABLE,
	BLOCK_TYPE_AIR_ASSIST_DISABLE,
	BLOCK_TYPE_AUX1_ASSIST_ENABLE,
	BLOCK_TYPE_AUX1_ASSIST_DISABLE,
} BLOCK_TYPE;

// Raster structure, used by gcode, planner and stepper.
typedef struct _raster {
	uint8_t *buffer;		// Raster data buffer
	uint32_t length;

	uint8_t intensity;
	uint8_t invert;
	double bidirectional;

	double dot_size;
	double x;
	double y;
} raster_t;

#define planner_control_air_assist_enable() planner_command(BLOCK_TYPE_AIR_ASSIST_ENABLE)
#define planner_control_air_assist_disable() planner_command(BLOCK_TYPE_AIR_ASSIST_DISABLE)
#define planner_control_aux1_assist_enable() planner_command(BLOCK_TYPE_AUX1_ASSIST_ENABLE)
#define planner_control_aux1_assist_disable() planner_command(BLOCK_TYPE_AUX1_ASSIST_DISABLE)

// This struct is used when buffering the setup for each linear movement "nominal" values are as specified in 
// the source g-code and may never actually be reached if acceleration management is active.
typedef struct {
  BLOCK_TYPE block_type;              // Type of command, eg: TYPE_LINE, TYPE_AIR_ASSIST_ENABLE
  // Fields used by the bresenham algorithm for tracing the line
  uint32_t steps_x, steps_y, steps_z; // Step count along each axis
  uint8_t  direction_bits;            // The direction bit set for this block (refers to *_DIRECTION_BIT in config.h)
  int32_t  step_event_count;          // The number of step events required to complete this block
  uint32_t nominal_rate;              // The nominal step rate for this block in step_events/minute
  // Fields used by the motion planner to manage acceleration
  double nominal_speed;               // The nominal speed for this block in mm/min  
  double entry_speed;                 // Entry speed at previous-current junction in mm/min
  double vmax_junction;               // max junction speed (mm/min) based on angle between segments, accel and deviation settings
  double millimeters;                 // The total travel of this block in mm
  uint8_t laser_pwm;    			  // 0-255 is 0-100% percentage
  uint32_t laser_ppi;           	  // Number of pulses per inch
  uint32_t laser_mmpp;           	  // Number of mm per pulse (calculated from ppi)
  bool recalculate_flag;              // Planner flag to recalculate trapezoids on entry junction
  bool nominal_length_flag;           // Planner flag for nominal speed always reached
  // Settings for the trapezoid generator
  uint32_t initial_rate;              // The jerk-adjusted step rate at start of block  
  uint32_t final_rate;                // The minimal rate at exit
  int32_t rate_delta;                 // The steps/minute to add or subtract when changing speed (must be positive)
  uint32_t accelerate_until;          // The index of the step event on which to stop acceleration
  uint32_t decelerate_after;          // The index of the step event on which to start decelerating
  double acceleration;          	  // Acceleration speed (mm/min/min)
  raster_t raster;
} block_t;

// Initialize the motion plan subsystem      
void planner_init();

// Process a raster.
// Rasters can be +/- in the x or y directions (not z).
// raster and raster_len contain the pointer and length of buffer containing 0-255 PWM values for each dot.
void planner_raster(double x, double y, double z,
		            double feed_rate, double acceleration,
		            uint8_t nominal_laser_intensity,
		            raster_t *raster);

// Add a new linear movement to the buffer.
// x, y and z is the signed, absolute target position in millimeters.
// Feed rate specifies the speed of the motion.
void planner_line(double x, double y, double z,
		          double feed_rate, double acceleration,
		          uint8_t laser_pwm, uint16_t laser_ppi);

// Add a new piercing action, lasing at one spot.
void planner_dwell(double seconds, uint8_t nominal_laser_intensity);

// Add a non-motion command to the queue.
// Typical types are: TYPE_AIR_ASSIST_ENABLE, TYPE_AIR_ASSIST_DISABLE, ...
// This call is blocking when the block buffer is full.
void planner_command(uint8_t type);


int planner_blocks_available(void);

// Gets the current block. Returns NULL if buffer empty
block_t *planner_get_current_block();

// Called when the current block is no longer needed. Discards the block and makes the memory
// availible for new blocks.
void planner_discard_current_block();

// purge all command in the buffer
void planner_reset_block_buffer();


// Reset the position vector
void planner_set_position(double x, double y, double z);

// update to stepper position when steppers have been stopped
// called from the stepper code that executes the stop
void planner_request_position_update();

#endif
