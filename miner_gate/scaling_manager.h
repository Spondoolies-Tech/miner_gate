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

#ifndef _____DC2SCALINGME__45R_H____
#define _____DC2SCALINGME__45R_H____

#include "scaling_ac2dc.h"
#include "scaling_dc2dc.h"

void asic_down(HAMMER *a, int down = 1);
int asic_can_down(HAMMER *a);

int test_serial(int loopid);
void print_scaling();


#endif
