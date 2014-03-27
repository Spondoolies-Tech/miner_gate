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
#include "leds.h"


static LIGHT_MODE lm_green;
static LIGHT_MODE lm_yellow;
int green_on;
int yellow_on;





// 2 lamps - 0=green, 1=yellow
void set_light_on_off(int light_id, int on) {

  LIGHT_MODE* lm;
  int* ls;
  if (light_id==LIGHT_YELLOW) {
   ls = &yellow_on;
  } else {
   ls = &green_on;
  }
  if (*ls == on) {
    return;
  }

  FILE *f;
  if (light_id==LIGHT_YELLOW) {
  	f = fopen("/sys/class/gpio/gpio22/value", "w");
  } else {
    f = fopen("/sys/class/gpio/gpio51/value", "w");
  }

  if (on) {
	  fprintf(f, "1");
  } else {
    fprintf(f, "0");
  }
  *ls = on;
  fclose(f);
}


void leds_periodic_quater_second_led(int light_id,int counter) {
  LIGHT_MODE* lm = (light_id==LIGHT_GREEN)?&lm_green:&lm_yellow;
  int* l_on = (light_id==LIGHT_GREEN)?&green_on:&yellow_on;

  if (*lm == LIGHT_MODE_FAST_BLINK) {
    //printf("%d \n",(counter)%2);
    set_light_on_off(light_id, (counter)%2);
  } else if (*lm == LIGHT_MODE_SLOW_BLINK) {
   // printf("%d \n",(counter/4)%2);
    set_light_on_off(light_id, (counter/4)%2);
  }
}



void leds_periodic_quater_second() {
   static int counter=0;
   counter++;
  leds_periodic_quater_second_led(LIGHT_GREEN,counter);
  //leds_periodic_quater_second_led(LIGHT_YELLOW,counter);
}





// 2 lamps - 0=green, 1=yellow
void set_light(int light_id, LIGHT_MODE m) {

//  echo 1 > /sys/class/gpio/gpio22/value
  LIGHT_MODE* lm;
  int* ls;

  if (light_id==LIGHT_YELLOW) {
    lm = &lm_yellow;
    ls = &yellow_on;
  } else {
    lm = &lm_green;
    ls = &green_on;
  }
 

  if (*lm != m) {
    *lm = m;
    if (m == LIGHT_MODE_ON) {
  	  set_light_on_off(light_id, true);
    } else if (m == LIGHT_MODE_OFF) {
      set_light_on_off(light_id, false); 
    }
  }
}




void leds_init() {
  FILE *f;
  f = fopen("/sys/class/gpio/export", "w");
  fprintf(f, "22");
  fclose(f);
  f = fopen("/sys/class/gpio/gpio22/direction", "w");
  fprintf(f, "out");
  fclose(f);
  f = fopen("/sys/class/gpio/export", "w");
  fprintf(f, "51");
  fclose(f);
  f = fopen("/sys/class/gpio/gpio51/direction", "w");
  fprintf(f, "out");
  fclose(f);
}




