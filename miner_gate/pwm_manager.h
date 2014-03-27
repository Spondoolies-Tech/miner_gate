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

#ifndef _____PWMLIB_H__B__
#define _____PWMLIB_H__B__

#include <stdint.h>
#include <unistd.h>
#include <defines.h>

//#include "dc2dc.h"
// DONT USE UNLESS YOU KNOW WHAT YOU DOING
void kill_fan();
void init_pwm();
void set_fan_level(int fan_level_percent);



#endif
