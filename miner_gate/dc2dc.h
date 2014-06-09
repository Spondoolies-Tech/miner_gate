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


#ifndef _____DC2DC__45R_H____
#define _____DC2DC__45R_H____

#include "defines.h"
#include <stdint.h>
#include <unistd.h>


typedef enum {
  ASIC_VOLTAGE_0 ,
  ASIC_VOLTAGE_555 ,
  ASIC_VOLTAGE_585 ,
  ASIC_VOLTAGE_630 ,
  ASIC_VOLTAGE_675 ,
  ASIC_VOLTAGE_681 ,
  ASIC_VOLTAGE_687 ,
  ASIC_VOLTAGE_693 ,
  ASIC_VOLTAGE_700 , 
  ASIC_VOLTAGE_705 , 
  ASIC_VOLTAGE_710 , 
  ASIC_VOLTAGE_715 , 
  ASIC_VOLTAGE_720 ,
  ASIC_VOLTAGE_765 ,
  ASIC_VOLTAGE_790 ,
  ASIC_VOLTAGE_810 ,
  ASIC_VOLTAGE_COUNT 
} DC2DC_VOLTAGE;

#define VTRIM_MIN 0x0FFc4  // 0.635

//#define VTRIM_MEDIUM 0x0ffdd //
//#define VTRIM_MAX 0x10008  // 0.810

#define VTRIM_TO_VOLTAGE_MILLI(XX)    ((55500 + (XX-0x0FFc4)*(266))/100)  
#define VOLTAGE_TO_VTRIM_MILLI(XXX)    ((((XXX)*100-55500)/266) + 0x0FFc4)  // 

/*
#define VOLTAGE_ENUM_TO_MILIVOLTS(ENUM, VALUE)                                 \
  {                                                                            \
    int xxx[ASIC_VOLTAGE_COUNT] = { 0, 555, 585, 630, 675, 681, 687, 693, 700, \
                                    705, 710, 715, 720, 765, 790,     \
                                    810 };                                     \
    VALUE = xxx[ENUM];                                                         \
  }
*/

void dc2dc_select_loop(int loop, int *err) ;
void dc2dc_disable_dc2dc(int loop, int *err);
void dc2dc_enable_dc2dc(int loop, int *err);
void dc2dc_init();
int dc2dc_get_dcr_inductor_cat(int loop , bool raw=false);
int dc2dc_set_dcr_inductor_cat(int loop , int value, bool raw=false);

int dc2dc_get_current_16s_of_amper(int loop, int* overcurrent_err, int* overcurrent_warning ,uint8_t *temp ,int *err);


int update_dc2dc_current_temp_measurments(int loop, int* overcurrent_err,  int* overcurrent_warning);

// in takes 0.2second for voltage to be stable.
// Remember to wait after you exit this function
//void dc2dc_set_voltage(int loop, DC2DC_VOLTAGE v, int *err);
void dc2dc_set_vtrim(int loop, uint32_t vtrim, bool vmargin_75low, int *err);


// Returns value like 810, 765 etc`
int dc2dc_get_voltage(int loop, int *err);
int dc2dc_get_temp(int loop, int *err);

#endif
