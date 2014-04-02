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

#ifndef _____PLLLIB_H__B__
#define _____PLLLIB_H__B__

#include <stdint.h>
#include <unistd.h>

#include "nvm.h"

typedef struct {
  uint16_t m_mult;
  uint16_t fvco;
  uint16_t p_div;
} pll_frequency_settings;

#if 1
#define FREQ_225_0      { 30, 900, 2 }
#define FREQ_240_0      { 32, 960, 2 }
#define FREQ_255_0      { 34, 1020, 2 }
#define FREQ_270_0      { 36, 1080, 2 }
#define FREQ_285_0      { 38, 1140, 2 }
#define FREQ_300_0      { 40, 1200, 2 }
#define FREQ_315_0      { 42, 1260, 2 }
#define FREQ_330_0      { 44, 1320, 2 }
#define FREQ_345_0      { 46, 1380, 2 }
#define FREQ_360_0      { 48, 1440, 2 }
#else
#define FREQ_225_0      { 15, 450, 1 }
#define FREQ_240_0      { 16, 480, 1 }
#define FREQ_255_0      { 17, 510, 1 }
#define FREQ_270_0      { 18, 540, 1 }
#define FREQ_285_0      { 19, 570, 1 }
#define FREQ_300_0      { 20, 600, 1 }
#define FREQ_315_0      { 21, 630, 1 }
#define FREQ_330_0      { 22, 660, 1 }
#define FREQ_345_0      { 23, 690, 1 }
#define FREQ_360_0      { 24, 720, 1 }
#endif
#define FREQ_375_0      { 25, 750, 1 }
#define FREQ_390_0      { 26, 780, 1 }
#define FREQ_405_0      { 27, 810, 1 }
#define FREQ_420_0      { 28, 840, 1 }
#define FREQ_435_0      { 29, 870, 1 }
#define FREQ_450_0      { 30, 900, 1 }
#define FREQ_465_0      { 31, 930, 1 }
#define FREQ_480_0      { 32, 960, 1 }
#define FREQ_495_0      { 33, 990, 1 }
#define FREQ_510_0      { 34, 1020, 1 }
#define FREQ_525_0      { 35, 1050, 1 }
#define FREQ_540_0      { 36, 1080, 1 }
#define FREQ_555_0      { 37, 1110, 1 }
#define FREQ_570_0      { 38, 1140, 1 }
#define FREQ_585_0      { 39, 1170, 1 }
#define FREQ_600_0      { 40, 1200, 1 }
#define FREQ_615_0      { 41, 1230, 1 }
#define FREQ_630_0      { 42, 1260, 1 }
#define FREQ_645_0      { 43, 1290, 1 }
#define FREQ_660_0      { 44, 0, 1 }
#define FREQ_675_0      { 45, 0, 1 }
#define FREQ_690_0      { 46, 0, 1 }
#define FREQ_705_0      { 47, 0, 1 }
#define FREQ_720_0      { 48, 0, 1 }
// Remove later!!! TODO not true
/*
#define FREQ_735_0      { 49, 0, 1 }
#define FREQ_750_0      { 50, 0, 1 }
#define FREQ_765_0      { 51, 0, 1 }
#define FREQ_780_0      { 52, 0, 1 }
#define FREQ_795_0      { 53, 0, 1 }
*/
#define FREQ_735_0      { 49, 0, 1 }
#define FREQ_750_0      { 50, 0, 1 }
#define FREQ_765_0      { 51, 0, 1 }
#define FREQ_780_0      { 52, 0, 1 }
#define FREQ_795_0      { 53, 0, 1 }

#define FREQ_810_0      { 54, 0, 1 }
#define FREQ_825_0      { 55, 0, 1 }
#define FREQ_840_0      { 56, 0, 1 }
#define FREQ_855_0      { 57, 0, 1 }
#define FREQ_870_0      { 58, 0, 1 }





void disable_engines_all_asics();
void disable_engines_asic(int addr);
void enable_engines_asic(int addr, int engines_mask);
void set_pll(int addr, ASIC_FREQ freq, bool with_reset = true);
int enable_good_engines_all_asics_ok();
void disable_asic_forever_rt(int addr);
void enable_all_engines_all_asics();
//void set_asic_freq(int addr, ASIC_FREQ new_freq);

#endif
