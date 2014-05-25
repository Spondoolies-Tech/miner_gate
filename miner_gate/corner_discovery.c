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

#include <stdio.h>
#include <stdlib.h>
#include "corner_discovery.h"
#include "scaling_manager.h"
#include <spond_debug.h>
#include "hammer.h"
#include "hammer_lib.h"
#include "squid.h"
#include "pll.h"
#include <time.h>


void enable_voltage_freq(ASIC_FREQ f) {
  int l, h, i = 0;
  // for each enabled loop
  
  disable_engines_all_asics();
  for (l = 0; l < LOOP_COUNT; l++) {
    if (vm.loop[l].enabled_loop) {
      // Set voltage
      int err;
      // dc2dc_set_voltage(l, vm.loop_vtrim[l], &err);
      dc2dc_set_vtrim(l, vm.loop[l].dc2dc.loop_vtrim, vm.loop[l].dc2dc.loop_margin_low, &err);

      // for each ASIC
      for (h = 0; h < HAMMERS_PER_LOOP; h++, i++) {
        HAMMER *a = &vm.hammer[l * HAMMERS_PER_LOOP + h];
        // Set freq
        if (a->asic_present) {
          set_pll(a->address, f);
        }
      }
    }
  }
  enable_good_engines_all_asics_ok();
}




void set_safe_voltage_and_frequency() {
  enable_voltage_freq(ASIC_FREQ_405);
  enable_good_engines_all_asics_ok(); 
}

//
// Code to discover bad loops at runtime.
//
void good_loops_fast_test() {
  psyslog("GOOD LOOPS FAST TEST\n");

  uint32_t good_loops = 0;
  int i, ret = 0, success;
  // For flushing purposes.
  test_serial(-1);
  test_serial(-1);
  
  for (i = 0; i < LOOP_COUNT; i++) {
    if (vm.loop[i].enabled_loop) {
      unsigned int bypass_loops = (~(1 << i) & 0xFFFFFF);
      write_spi(ADDR_SQUID_LOOP_BYPASS, bypass_loops);
      if (!test_serial(i)) {
        vm.loop[i].enabled_loop = 0;
        vm.loop[i].dc2dc.max_vtrim_currentwise = 0;
        vm.loop[i].dc2dc.loop_vtrim = 0;
        vm.loop[i].dc2dc.loop_margin_low = true;        
        for (int h = i * HAMMERS_PER_LOOP; h < (i + 1) * HAMMERS_PER_LOOP; h++) {
          vm.hammer[h].asic_present = 0;
          vm.hammer[h].working_engines = 0;
        }
        int err;
        psyslog("BAD LOOP %d\n", i);
        vm.good_loops = vm.good_loops & ~(1<<i);
        ret++;
        //dc2dc_disable_dc2dc(i, &err);
      }
    }
  }
  write_spi(ADDR_SQUID_LOOP_BYPASS, ~(vm.good_loops));
  
  test_serial(-1); 
  psyslog("Found %d bad loops\n", ret);

}


void discover_good_loops() {
  DBG(DBG_NET, "RESET SQUID\n");

  uint32_t good_loops = 0;
  int i, ret = 0, success;

  write_spi(ADDR_SQUID_LOOP_BYPASS, 0xFFFFFF);
  write_spi(ADDR_SQUID_LOOP_RESET, 0);
  write_spi(ADDR_SQUID_COMMAND, 0xF);
  write_spi(ADDR_SQUID_COMMAND, 0x0);
  write_spi(ADDR_SQUID_LOOP_BYPASS, 0);
  write_spi(ADDR_SQUID_LOOP_RESET, 0xffffff);

 
  for (i = 0; i < LOOP_COUNT; i++) {
    vm.loop[i].id = i;
    vm.loop[i].dc2dc.last_downscale_time = time(NULL);
    unsigned int bypass_loops = (~(1 << i) & 0xFFFFFF);
    write_spi(ADDR_SQUID_LOOP_BYPASS, bypass_loops);
    if (vm.loop[i].enabled_loop && test_serial(i)) {
      vm.loop[i].enabled_loop = 1;
      vm.loop[i].dc2dc.max_vtrim_currentwise = vm.vtrim_max;
      vm.loop[i].dc2dc.loop_vtrim = vm.vtrim_start;
      vm.loop[i].dc2dc.loop_margin_low = vm.vmargin_start;
      vm.loop[i].dc2dc.dc_current_limit_16s = vm.max_dc2dc_current_16s;
      good_loops |= 1 << i;
      ret++;
    } else {
      vm.loop[i].enabled_loop = 0;
      vm.loop[i].dc2dc.max_vtrim_currentwise = 0;
      vm.loop[i].dc2dc.loop_vtrim = 0;
      for (int h = i * HAMMERS_PER_LOOP; h < (i + 1) * HAMMERS_PER_LOOP; h++) {
        vm.hammer[h].asic_present = 0;
        vm.hammer[h].working_engines = 0;
      }
      int err;
      printf("Disabling DC2DC %d\n", i);
      dc2dc_disable_dc2dc(i, &err);
    }
  }
  write_spi(ADDR_SQUID_LOOP_BYPASS, ~(good_loops));
  vm.good_loops = good_loops;
  test_serial(-1); 
  psyslog("Found %d good loops\n", ret);
  passert(ret);
}



