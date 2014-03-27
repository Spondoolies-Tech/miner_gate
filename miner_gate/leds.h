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

#ifndef _____HAMMER_LEDS_H____
#define _____HAMMER_LEDS_H____

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include "spond_debug.h"



void leds_init();
#define LIGHT_GREEN  0
#define LIGHT_YELLOW 1

typedef enum {
  LIGHT_MODE_OFF = 0,
  LIGHT_MODE_ON = 1,
  LIGHT_MODE_SLOW_BLINK = 2,
  LIGHT_MODE_FAST_BLINK = 3,
} LIGHT_MODE;


void set_light(int light_id, LIGHT_MODE m);
void leds_periodic_quater_second();




#endif
