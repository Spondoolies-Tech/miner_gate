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

#ifndef _____DC2DCSCALINGME__45R_H____
#define _____DC2DCSCALINGME__45R_H____
#include "defines.h"


void decrease_asics_freqs();
int asic_can_down(HAMMER *a);
int asic_can_up(HAMMER *a,  int force);
void asic_up(HAMMER *a);

// Called each second.
void change_dc2dc_voltage_if_needed();
void do_bist_fix_loops_rt();
void pause_asics_if_needed();
void asic_frequency_update_nrt(int verbal = 0);
void resume_asics_if_needed();


#endif
