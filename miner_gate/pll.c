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

#include "pll.h"
#include "hammer.h"
#include "defines.h"
#include "squid.h"
#include "hammer_lib.h"
#include "scaling_manager.h"
#include "spond_debug.h"


pll_frequency_settings pfs[ASIC_FREQ_MAX] = {
  { 0, 0, 0 }, 
  FREQ_225_0, FREQ_240_0, FREQ_255_0, FREQ_270_0, FREQ_285_0,
  FREQ_300_0, FREQ_315_0, FREQ_330_0, FREQ_345_0, FREQ_360_0, FREQ_375_0,
  FREQ_390_0, FREQ_405_0, FREQ_420_0, FREQ_435_0, FREQ_450_0, FREQ_465_0,
  FREQ_480_0, FREQ_495_0, FREQ_510_0, FREQ_525_0, FREQ_540_0, FREQ_555_0,
  FREQ_570_0, FREQ_585_0, FREQ_600_0, FREQ_615_0, FREQ_630_0, FREQ_645_0,
  FREQ_660_0, FREQ_675_0, FREQ_690_0, FREQ_705_0, FREQ_720_0, FREQ_735_0,
  FREQ_750_0, FREQ_765_0, FREQ_780_0, FREQ_795_0, FREQ_810_0, FREQ_825_0, 
  FREQ_840_0, FREQ_855_0, FREQ_870_0, 
};



void enable_all_engines_all_asics() {
  int i;
  write_reg_broadcast(ADDR_RESETING0, ALL_ENGINES_BITMASK);
  write_reg_broadcast(ADDR_RESETING1, ALL_ENGINES_BITMASK | 0x8000);
  write_reg_broadcast(ADDR_ENABLE_ENGINE, ALL_ENGINES_BITMASK);
  flush_spi_write();
}



void disable_engines_all_asics() {
  //printf("Disabling engines:\n");
  write_reg_broadcast(ADDR_ENABLE_ENGINE, 0);
  write_reg_broadcast(ADDR_RESETING1, 0);
  write_reg_broadcast(ADDR_RESETING0, 0);
  write_reg_broadcast(ADDR_CLK_ENABLE, 0);
  flush_spi_write();
  vm.engines_disabled = 1;
}


void disable_engines_asic(int addr) {
  //printf("Disabling engines:\n");
  write_reg_device(addr,ADDR_ENABLE_ENGINE, 0);
  write_reg_device(addr,ADDR_RESETING1, 0);
  write_reg_device(addr,ADDR_RESETING0, 0);
  write_reg_device(addr,ADDR_CLK_ENABLE, 0);
  //flush_spi_write();
}


void enable_engines_asic(int addr, int engines_mask) {
  //printf("Disabling engines:\n");
  write_reg_device(addr,ADDR_CLK_ENABLE, engines_mask);  
  write_reg_device(addr,ADDR_ENABLE_ENGINE, engines_mask);
  write_reg_device(addr,ADDR_RESETING1, engines_mask | 0x8000);
  write_reg_device(addr,ADDR_RESETING0, engines_mask);
  //flush_spi_write();
}




void set_pll(int addr, ASIC_FREQ freq) {
  //passert(vm.engines_disabled == 1);
  write_reg_device(addr, ADDR_DLL_OFFSET_CFG_LOW, 0xC3C1C200);
  write_reg_device(addr, ADDR_DLL_OFFSET_CFG_HIGH, 0x0082C381);
  passert(freq < ASIC_FREQ_MAX);
  passert(freq >= ASIC_FREQ_225);
  pll_frequency_settings *ppfs;
  ppfs = &pfs[freq];
  uint32_t pll_config = 0;
  uint32_t M = ppfs->m_mult;
  uint32_t P = ppfs->p_div;
  pll_config = (M - 1) & 0xFF;
  pll_config |= ((P - 1) & 0x1F) << 13;
  pll_config |= 0x100000;
  //printf("Pll %d %x->%x\n",addr,vm.hammer[addr].freq_hw, freq);
  write_reg_device(addr, ADDR_PLL_CONFIG, pll_config);
  write_reg_device(addr, ADDR_PLL_ENABLE, 0x0);
  write_reg_device(addr, ADDR_PLL_ENABLE, 0x1);
  vm.hammer[addr].freq_hw = freq;
  //printf("Sett pll %x", addr);
}

void disable_asic_forever_rt(int addr) {
  vm.hammer[addr].working_engines = 0;
  vm.hammer[addr].asic_present = 0;
  disable_engines_asic(addr);
  psyslog("Disabing ASIC forever %x from loop %d\n", addr, addr/HAMMERS_PER_LOOP);
  write_reg_device(addr, ADDR_CONTROL_SET1, BIT_CTRL_DISABLE_TX);
  vm.loop[vm.hammer[addr].loop_address].asic_count--;
}


int enable_good_engines_all_asics_ok() {
    int i = 0; 
    int reg;
    int killed_pll=0;
    
    while ((reg = read_reg_broadcast(ADDR_BR_PLL_NOT_READY)) != 0) {
      if (i++ > 100) {
        psyslog(RED "PLL %x stuck, killing ASIC\n" RESET, reg);
        //return 0;
        int addr = BROADCAST_READ_ADDR(reg);
        disable_asic_forever_rt(addr);
        killed_pll++;
      }
      usleep(10);
    }
   //printf("Enabling engines from NVM:\n");
   if (killed_pll) {
     passert(test_serial(-1)); 
   }

   //passert(vm.engines_disabled == 1);
   write_reg_broadcast(ADDR_CLK_ENABLE, ALL_ENGINES_BITMASK);
   write_reg_broadcast(ADDR_RESETING0, ALL_ENGINES_BITMASK);
   write_reg_broadcast(ADDR_RESETING1, ALL_ENGINES_BITMASK | 0x8000);
   write_reg_broadcast(ADDR_ENABLE_ENGINE, ALL_ENGINES_BITMASK);
   flush_spi_write();
 

   for (int h = 0; h < HAMMERS_COUNT ; h++) {
     if (vm.loop[h/HAMMERS_PER_LOOP].enabled_loop &&
         !vm.hammer[h].asic_present) {
       write_reg_device(h, ADDR_CLK_ENABLE, 0);
       write_reg_device(h, ADDR_RESETING0, 0);
       write_reg_device(h, ADDR_RESETING1, 0);
       write_reg_device(h, ADDR_ENABLE_ENGINE, 0);
     } 

     if (vm.hammer[h].asic_present && 
        vm.hammer[h].working_engines != ALL_ENGINES_BITMASK) {
        //printf("Working engines %x to %d\n",vm.hammer[h].working_engines,h);
        write_reg_device(h, ADDR_CLK_ENABLE, vm.hammer[h].working_engines);
        write_reg_device(h, ADDR_RESETING0, vm.hammer[h].working_engines);
        write_reg_device(h, ADDR_RESETING1, vm.hammer[h].working_engines | 0x8000);
        write_reg_device(h, ADDR_ENABLE_ENGINE, vm.hammer[h].working_engines);
     }
   }
  
   vm.engines_disabled = 0;
   return 1;
}



/*
void set_asic_freq(int addr, ASIC_FREQ new_freq) {
  if (vm.hammer[addr].freq_hw != new_freq) {
    //printf("Changes ASIC %x frequency from %d to %d\n", addr,vm.hammer[addr].freq*15+210,new_freq*15+210);
    assert(new_freq >= MINIMAL_ASIC_FREQ);
    vm.hammer[addr].freq_wanted = new_freq;
    vm.hammer[addr].freq_hw = new_freq;
  }
  set_pll(addr, new_freq, false);
}
*/



