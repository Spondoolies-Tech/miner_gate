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
 

#include "pwm_manager.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <spond_debug.h>
#include "hammer.h"
#include "hammer_lib.h"
#include "pthread.h"

extern pthread_mutex_t i2cm;




// XX in percent - 0 to 100
// #define PWM_VALUE(XX)  (12000+XX*(320-120)) 
#define PWM_VALUE(XX)  (12000+XX*(200))  // TODO - can go up to 250
//int pwm_values[] = { 12000 , 22000, 32000 };
void init_pwm() {
  // cd /sys/devices/bone_capemgr.*

  FILE *f;
  f = fopen("/sys/devices/bone_capemgr.9/slots", "w");
  fprintf(f, "am33xx_pwm");
  fclose(f);
  f = fopen("/sys/devices/bone_capemgr.9/slots", "w");
  fprintf(f, "bone_pwm_P9_31");
  fclose(f);

  f = fopen("/sys/devices/ocp.3/pwm_test_P9_31.12/period", "w");
  passert(f != NULL);
  fprintf(f, "40000");
  fclose(f);
  // pwm_values
  f = fopen("/sys/devices/ocp.3/pwm_test_P9_31.12/duty", "w");
  passert(f != NULL);
  fprintf(f, "%d", PWM_VALUE(0));
  fclose(f);
}

void kill_fan() {
  FILE *f;
  int val = 0;
  f = fopen("/sys/devices/ocp.3/pwm_test_P9_31.12/duty", "w");
  if (f <= 0) {
    psyslog(RED "Fan PWM not found\n" RESET);
  } else {
    fprintf(f, "%d", val);
    fclose(f);
  }
}


void set_fan_level(int fan_level) {
  FILE *f;
  if (vm.fan_level != fan_level) {
	  passert(fan_level <= 100 && fan_level >=0);
	  int val = PWM_VALUE(fan_level);
	  
	  if (vm.silent_mode) {
		  val = 20;
	  }
	  f = fopen("/sys/devices/ocp.3/pwm_test_P9_31.12/duty", "w");
	  if (f <= 0) {
	    psyslog(RED "Fan PWM not found\n" RESET);
	  } else {
	    fprintf(f, "%d", val);
	    fclose(f);
	    vm.fan_level = fan_level;
	  }
  	}
}

