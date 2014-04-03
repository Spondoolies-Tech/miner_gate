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

#include "squid.h"
#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <netinet/in.h>
#include "queue.h"
#include "pll.h"
#include "spond_debug.h"
#include "hammer.h"
#include <sys/time.h>
#include "nvm.h"
#include "ac2dc.h"
#include "dc2dc.h"
#include "hammer_lib.h"
#include "defines.h"

#include "scaling_manager.h"
#include "corner_discovery.h"

static int now; // cahce time
extern int kill_app;

int loop_can_down(int l) {
  if (l == -1)
      return 0;
  
  return  
     (vm.loop[l].enabled_loop && 
     (vm.loop_vtrim[l] > VTRIM_MIN));
}


void loop_down(int l) {
  int err;
   //printf("vtrim=%x\n",vm.loop_vtrim[l]);
   psyslog( "LOOP DOWN:%d\n" , l);
   dc2dc_set_vtrim(l, vm.loop_vtrim[l]-1, vm.loop_margin_low[l], &err);
   vm.loop[l].last_ac2dc_scaling_on_loop  = now;
   for (int h = l*HAMMERS_PER_LOOP; h < l*HAMMERS_PER_LOOP+HAMMERS_PER_LOOP;h++) {
      if (vm.hammer[h].asic_present) {
        // learn again
        vm.hammer[h].freq_thermal_limit = vm.hammer[h].freq_bist_limit;
        /*
        vm.hammer[h].freq_wanted = vm.hammer[h].freq_wanted - 1;
        if (vm.hammer[h].freq_wanted > ASIC_FREQ_225) {
            vm.hammer[h].freq_wanted = vm.hammer[h].freq_wanted - 1;
        }
        */
      }
   }
}



int loop_can_up(int l) {
  if (l == -1)
      return 0;
  
  return  
    (vm.loop[l].enabled_loop &&
    (vm.loop_vtrim[l] < VTRIM_HIGH) &&
    (vm.loop_vtrim[l] < vm.loop[l].dc2dc.max_vtrim_currentwise) &&
    ((now - vm.loop[l].last_ac2dc_scaling_on_loop) > AC2DC_SCALING_SAME_LOOP_PERIOD_SECS));
 
}


void loop_up(int l) {
  int err;  
  //printf("1\n");
  dc2dc_set_vtrim(l, vm.loop_vtrim[l]+1, vm.loop_margin_low[l], &err);
  vm.loop[l].last_ac2dc_scaling_on_loop  = now;
   //printf("3\n");
   psyslog( "LOOP UP:%d\n" , l);
  for (int h = l*HAMMERS_PER_LOOP; h< l*HAMMERS_PER_LOOP+HAMMERS_PER_LOOP;h++) {
    if (vm.hammer[h].asic_present) {
          // if the limit is bist limit, then let asic grow a bit more
          // if its termal, dont change it.
          if (vm.hammer[h].freq_bist_limit == vm.hammer[h].freq_thermal_limit) {
            vm.hammer[h].freq_thermal_limit = vm.hammer[h].freq_bist_limit = 
              (vm.hammer[h].freq_bist_limit < ASIC_FREQ_MAX-1)?((ASIC_FREQ)(vm.hammer[h].freq_bist_limit+1)):ASIC_FREQ_MAX; 
          }
          vm.hammer[h].agressivly_scale_up = true;
        } 
    }
}



#if 0

int asic_frequency_update_nrt_fast() {    
  pause_asics_if_needed();
  int one_ok = 0;
  for (int l = 0 ; l < LOOP_COUNT ; l++) {
    if (!vm.loop[l].enabled_loop) {
      continue;
    }
    //printf("DOING INIT BIST ON FREQ: %d\n", vm.hammer[l*HAMMERS_PER_LOOP].freq_wanted );
    for (int a = 0 ; a < HAMMERS_PER_LOOP; a++) {
      HAMMER *h = &vm.hammer[l*HAMMERS_PER_LOOP+a];
      if (!h->asic_present) {
        continue;
      }
      int passed = h->passed_last_bist_engines;        
      if ((passed == ALL_ENGINES_BITMASK)) {
          one_ok = 1;
          h->freq_wanted = (ASIC_FREQ)(h->freq_wanted+1);
          h->freq_thermal_limit = h->freq_wanted;
          h->freq_bist_limit = h->freq_wanted;    
          set_pll(h->address, h->freq_wanted);  
      } else if (h->freq_wanted == ASIC_FREQ_225) {
          h->working_engines = h->working_engines&passed;
          h->freq_wanted = (ASIC_FREQ)(h->freq_wanted+1);
          h->freq_thermal_limit = h->freq_wanted;
          h->freq_bist_limit = h->freq_wanted;    
          set_pll(h->address, h->freq_wanted);  
          h->passed_last_bist_engines = ALL_ENGINES_BITMASK;
      } else {      

        h->passed_last_bist_engines = ALL_ENGINES_BITMASK;
      }
    }
  }
  return one_ok; 
}


#else


int asic_frequency_update_nrt_fast() {    
  pause_asics_if_needed();
  int one_ok = 0;
  for (int l = 0 ; l < LOOP_COUNT ; l++) {
    if (!vm.loop[l].enabled_loop) {
      continue;
    }
    //printf("DOING INIT BIST ON FREQ: %d\n", vm.hammer[l*HAMMERS_PER_LOOP].freq_wanted );
    for (int a = 0 ; a < HAMMERS_PER_LOOP; a++) {
      HAMMER *h = &vm.hammer[l*HAMMERS_PER_LOOP+a];
      if (!h->asic_present || h->initial_bist_done) {
        continue;
      }
      
      int passed = h->passed_last_bist_engines;        
      if ((passed == ALL_ENGINES_BITMASK)) {
          //PASSED BIST
          printf("P:%d[%d] ", h->address, h->freq_wanted*15+210);
          one_ok = 1;
          h->freq_wanted = (ASIC_FREQ)(h->freq_wanted+1);
          if (h->freq_wanted == ASIC_FREQ_MAX) {
            disable_asic_forever_rt(h->address);
            continue;
          }
          h->freq_thermal_limit = h->freq_wanted;
          h->freq_bist_limit = h->freq_wanted;
          set_pll(h->address, h->freq_wanted);
      } else if (h->freq_wanted == ASIC_FREQ_225) {
          // FAILED BIST at FREQ 225
          printf("X:%d[%d] (%x)", h->address, h->freq_wanted*15+210, passed);
          h->working_engines = h->working_engines&passed;
          one_ok = 1;
          h->freq_wanted = (ASIC_FREQ)(h->freq_wanted+1);
          h->freq_thermal_limit = h->freq_wanted;
          h->freq_bist_limit = h->freq_wanted;
          set_pll(h->address, h->freq_wanted);
      } else {
          // take one before last BIST.
          printf("F:%d[%d] (%x)", h->address, h->freq_wanted*15+210, passed);
          // Keep it one above :)
          h->freq_wanted = (ASIC_FREQ)(h->freq_wanted-1);
          h->freq_thermal_limit = h->freq_wanted;
          h->freq_bist_limit = h->freq_wanted;    
          set_pll(h->address, h->freq_wanted);  
          h->initial_bist_done = 1;
      }
      h->passed_last_bist_engines = ALL_ENGINES_BITMASK;
    }
  }
  return one_ok; 
}

#endif

void set_working_voltage_discover_top_speeds() {
  int one_ok;
  enable_voltage_freq(ASIC_FREQ_225);
  do {
    // Let'em plls lock proper
    //usleep(10000);
    resume_asics_if_needed();
    //usleep(10000);    
    do_bist_ok_rt(0);
    one_ok = asic_frequency_update_nrt_fast();
 } while (one_ok && (!kill_app));


  // All remember BIST they failed!
  for (int h =0; h < HAMMERS_COUNT ; h++) {
    if (vm.hammer[h].asic_present) {
      /*
       vm.hammer[h].freq_hw = (ASIC_FREQ)(vm.hammer[h].freq_hw - 1);
       vm.hammer[h].freq_wanted = (ASIC_FREQ)(vm.hammer[h].freq_wanted - 1);
       vm.hammer[h].freq_thermal_limit = (ASIC_FREQ)(vm.hammer[h].freq_thermal_limit - 1);
       vm.hammer[h].freq_bist_limit = (ASIC_FREQ)(vm.hammer[h].freq_bist_limit - 1);      
       */
       vm.hammer[h].passed_last_bist_engines = ALL_ENGINES_BITMASK;
    }
  }

  resume_asics_if_needed();
}






void ac2dc_scaling_loop(int l) {
  int changed = 0;
  now=time(NULL);
  if ((!vm.asics_shut_down_powersave) && 
       (vm.loop[l].enabled_loop) &&
       (vm.cosecutive_jobs >= MIN_COSECUTIVE_JOBS_FOR_SCALING)) {
  
   // int temperature_high = (vm.loop[l].asic_temp_sum / vm.loop[l].asic_count >= 113);
   // int fully_utilized = (vm.loop[l].overheating_asics == 0); // h->freq_thermal_limit - h->freq
    int tmp_scaled=0;
   
   
    //printf(CYAN "ac2dc %d %d %d!\n" RESET, l, vm.loop[l].overheating_asics, free_current);
    printf(CYAN "%d vm.loop[l].overheating_asics:%d \n" RESET, l, vm.loop[l].overheating_asics);
  
    
     // has unused freq - scale down.
     if (vm.loop[l].overheating_asics >= 6) {
        if (loop_can_down(l)) {
          psyslog( "LOOP DOWN:%d\n" , l);            
          changed = 1;
          loop_down(l);  
          vm.ac2dc_power -= 2;
        }
     } else if ((vm.max_ac2dc_power - vm.ac2dc_power) > 5 &&
                 (vm.loop[l].overheating_asics < 4) && 
                 (vm.loop[l].crit_temp_downscale < 500)) {
    // scale up
      if (loop_can_up(l)) {          
        printf( "LOOP UP:%d\n" , l);
        changed = 1;
        loop_up(l);
        vm.ac2dc_power += 3;
      }
    }    
  }
  vm.loop[l].dc2dc.last_voltage_change_time = time(NULL);


}




// Called from low-priority thread.
void ac2dc_scaling() {
  // First raws
  for (int l = 0; l < 4; l++) {
    ac2dc_scaling_loop(l);
  }

  for (int l = 12; l < 16; l++) {
    ac2dc_scaling_loop(l);
  }

  // Second raws
  for (int l = 4; l < 8; l++) {
    ac2dc_scaling_loop(l);
  }

  for (int l = 16; l < 20; l++) {
    ac2dc_scaling_loop(l);
  }

  // Third raws
  for (int l = 8; l < 12; l++) {
    ac2dc_scaling_loop(l);
  }

  for (int l = 20; l < LOOP_COUNT; l++) {
    ac2dc_scaling_loop(l);
  }
}


