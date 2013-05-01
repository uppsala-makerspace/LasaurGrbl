/*
  temperature.h - Read temperature from 1 to 3 DS18X20 sensors
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
#ifndef _temperature_h

void temperature_init(void);
uint8_t temperature_num_sensors(void);
uint16_t temperature_read(uint8_t sensor);

#endif /* _temperature_h */
