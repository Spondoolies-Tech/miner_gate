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

#ifndef _____DC2DEFINES__45R_H____
#define _____DC2DEFINES__45R_H____

// compilation flags
#define TEST_BOARD 0
#define ECONOMY 0
//#define NO_PEAKS 

// System parameters
// Above this not allowed
#define MAX_ASIC_TEMPERATURE ASIC_TEMP_119

#define ASIC_TO_SET_FANS_HIGH_COUNT   50
#define ASIC_TO_SET_FANS_LOW_COUNT    20


#define MAX_FAN_LEVEL                   100
#define HOT_ASICS_IN_LOOP_FOR_DOWNSCALE 6
#define COLD_ASICS_IN_LOOP_FOR_UPSCALE  4


#define TIME_FOR_DLL_USECS  1000
#define MIN_COSECUTIVE_JOBS_FOR_SCALING 400
#define MAX_CONSECUTIVE_JOBS_TO_COUNT (800)
#define MIN_COSECUTIVE_JOBS_FOR_AC2DC_MEASUREMENT MIN_COSECUTIVE_JOBS_FOR_SCALING
#define MIN_COSECUTIVE_JOBS_FOR_DC2DC_MEASUREMENT MIN_COSECUTIVE_JOBS_FOR_SCALING

// In seconds
#define BIST_PERIOD_SECS                              20 
#define MAX_LEADING_ZEROES                            62

#define TRY_ASIC_FREQ_INCREASE_PERIOD_SECS 2

#define MAX_BOTTOM_TEMP 90
#define MAX_TOP_TEMP 90
#define MAX_MGMT_TEMP 50


#define AC2DC_TEMP_GREEN_LINE 110
#define AC2DC_CURRENT_TRUSTWORTHY (10)
#define AC2DC_POWER_LIMIT  (1300)
#define AC2DC_UPSCALE_TIME_SECS   60  //2 2 minutes wait before upscaling AC2DC 

#define AC2DC_CURRENT_JUMP_16S        (16)
#define AC2DC_CURRENT_MINIMAL_FOR_DOWNSCALE_16S        (16)
#define AC2DC_SCALING_SAME_LOOP_PERIOD_SECS  5 



#define DC2DC_TEMP_GREEN_LINE         120
#define DC2DC_INITIAL_CURRENT_16S (61 * 16) 

/*
#define VTRIM_658   0xFFCE
#define VTRIM_661   0xFFCF
#define VTRIM_664   0xFFD0
#define VTRIM_666   0xFFD1
#define VTRIM_669   0xFFD2
#define VTRIM_678   0xFFD6
#define VTRIM_676   0xFFD5
#define VTRIM_674   0xFFD4
#define VTRIM_671   0xFFD3
#define VTRIM_669   0xFFD2
#define VTRIM_706   0xFFE0
#define VTRIM_706   0xFFE0
#define VTRIM_810   0x10008
#define VTRIM_680   0xffd6
#define VTRIM_683   0xffd7
#define VTRIM_685   0xffd8
#define VTRIM_688   0xffd9
*/

#define VTRIM_ML_630   0xFFE1
#define VTRIM_ML_600   0xFFD4
#define VTRIM_ML_585   0xFFCF
#define VTRIM_ML_555   0xFFC4


#define COUNT_IDLES 0
#define VTRIM_HIGH  0x1001E


//#define CORNER_DISCOVERY_FREQ_SS       ASIC_FREQ_480 // all less


#define TOO_HOT_COUNER_DISABLE_ASIC   20


#define MAX_ASIC_FREQ ASIC_FREQ_840
#define MINIMAL_ASIC_FREQ ASIC_FREQ_225

#define AC2DC_BUG    1
#define IDLE_TIME_TO_PAUSE_ENGINES 30

#endif
