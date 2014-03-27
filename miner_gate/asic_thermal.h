/*
 * Copyright 2014 Zvi (Zvisha) Shteingart - Spondoolies-tech.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.  See COPYING for more details.
 *
 * Note that changing this SW will void your miners guaranty
 */

#ifndef _____DCASSTHERMALIXT__45R_H____
#define _____DCASSTHERMALIXT__45R_H____

#include "defines.h"

void read_some_asic_temperatures(int loop, ASIC_TEMP temp_measure_temp0, ASIC_TEMP temp_measure_temp1);
void thermal_init();
int read_asic_temperature(int addr, ASIC_TEMP temp_measure_temp0, ASIC_TEMP temp_measure_temp1);
void enable_thermal_shutdown();




#endif
