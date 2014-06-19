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
#include <stdlib.h>
#include <linux/types.h>
#include <unistd.h>
#include "mg_proto_parser.h"
#include "hammer.h"
#include "queue.h"
#include <string.h>
#include "ac2dc.h"
#include "hammer_lib.h"
#include "asic_thermal.h"

#include <spond_debug.h>
#include <sys/time.h>
#include "real_time_queue.h"
#include "miner_gate.h"
#include "scaling_manager.h"

//#if TEST_BOARD


void enable_thermal_shutdown() {
/*
  write_reg_broadcast(ADDR_TS_EN_0, 1);
  write_reg_broadcast(ADDR_TS_EN_1, 1);
  write_reg_broadcast(ADDR_SHUTDOWN_ACTION, ALL_ENGINES_BITMASK | 0x78);  
  write_reg_broadcast(ADDR_TS_RSTN_0, 0);
  write_reg_broadcast(ADDR_TS_RSTN_1, 0);
  write_reg_broadcast(ADDR_TS_SET_0, (ASIC_TEMP_125- 1) | 
                          ADDR_TS_SET_THERMAL_SHUTDOWN_ENABLE);
  write_reg_broadcast(ADDR_TS_SET_1, (ASIC_TEMP_125- 1) | 
                          ADDR_TS_SET_THERMAL_SHUTDOWN_ENABLE);
  write_reg_broadcast(ADDR_TS_RSTN_0, 1);
  write_reg_broadcast(ADDR_TS_RSTN_1, 1);
  flush_spi_write();
*/
}



void thermal_init() {
  write_reg_broadcast(ADDR_TS_EN_0, 1);
  write_reg_broadcast(ADDR_TS_EN_1, 1);
  write_reg_broadcast(ADDR_SHUTDOWN_ACTION, ALL_ENGINES_BITMASK | 0x78);  
  write_reg_broadcast(ADDR_TS_RSTN_0, 0);
  write_reg_broadcast(ADDR_TS_RSTN_1, 0);
  // Set to vm.max_asic_temp-1 
  write_reg_broadcast(ADDR_TS_SET_0, (vm.max_asic_temp-2));
  // Set to vm.max_asic_temp
  write_reg_broadcast(ADDR_TS_SET_1, (vm.max_asic_temp-1));
  write_reg_broadcast(ADDR_TS_RSTN_0, 1);
  write_reg_broadcast(ADDR_TS_RSTN_1, 1);
  flush_spi_write();
  
}

// Returns bitmask temp_measure_temp0 | temp_measure_temp1<1
int read_asic_temperature(int addr, ASIC_TEMP temp_measure_temp0, ASIC_TEMP temp_measure_temp1) {
  int ret = 0; 
  write_reg_device(addr, ADDR_TS_SET_0, temp_measure_temp0-1);
  write_reg_device(addr, ADDR_TS_SET_1, temp_measure_temp1-1);
  write_reg_device(addr, ADDR_COMMAND, BIT_CMD_TS_RESET_1 | BIT_CMD_TS_RESET_0);
  int res = read_reg_device(addr, ADDR_INTR_RAW);
  return res;
  if (res & BIT_INTR_0_OVER) { 
    ret |= 1;
  }
  if (res & BIT_INTR_1_OVER) { 
    ret |= 2;
  }
  return ret;
}


 
#if 0
void read_some_asic_temperatures(int loop, 
                                 ASIC_TEMP temp_measure_temp0, 
                                 ASIC_TEMP temp_measure_temp1) {
  if (!vm.loop[loop].enabled_loop) {
    return;
  }
  
  uint32_t val[HAMMERS_PER_LOOP];
  // printf("T:%d L:%d\n",temp_measure_temp, loop);

  // Push temperature measurment to the whole loop
  // read_reg_device(uint16_t cpu,uint8_t offset)
  write_reg_broadcast(ADDR_TS_SET_0, temp_measure_temp0-1);
  write_reg_broadcast(ADDR_TS_SET_1, temp_measure_temp1-1);
  write_reg_broadcast(ADDR_COMMAND, BIT_CMD_TS_RESET_1 | BIT_CMD_TS_RESET_0);
  
  for (int h = 0; h < HAMMERS_PER_LOOP; h++) {
    HAMMER *a = &vm.hammer[loop * HAMMERS_PER_LOOP + h];
    val[h] = 0;
    if (a->asic_present) {
      push_hammer_read(a->address, ADDR_INTR_RAW, &(val[h]));
    }
  }
  squid_wait_hammer_reads();


  
  for (int h = 0; h < HAMMERS_PER_LOOP; h++) {
    HAMMER *a = &vm.hammer[loop * HAMMERS_PER_LOOP + h];
    if (a->asic_present) {
      if (val[h] & BIT_INTR_0_OVER) { 
        // This ASIC is at least this temperature high
        if (a->asic_temp < temp_measure_temp0) {
           // printf(GREEN "0+:ASIC %x Temp was %d now %d \n" RESET,a->address,
            //  a->asic_temp*6+77, temp_measure_temp0*6+77);
          a->asic_temp = temp_measure_temp0;
         
        }
      } else { 
        // ASIC is colder then this temp!
        if (a->asic_temp >= temp_measure_temp0) {
            //printf(GREEN "0-:ASIC %x Temp was %d now %d \n" RESET,a->address,
            //  a->asic_temp*6+77, (temp_measure_temp0-1)*6+77);
          a->asic_temp = (ASIC_TEMP)(temp_measure_temp0-1);
          
        }
      }

      if (val[h] & BIT_INTR_1_OVER) { 
         // This ASIC is at least this temperature high
         if (a->asic_temp < temp_measure_temp1) {
           // printf(GREEN "1+:ASIC %x Temp was %d now %d \n" RESET,a->address,
            //  a->asic_temp*6+77, temp_measure_temp1*6+77);
           a->asic_temp = temp_measure_temp1;
         
         }
       } else { 
         // ASIC is colder then this temp!
         if (a->asic_temp >= temp_measure_temp1) {
           // (GREEN "1-:ASIC %x Temp was %d now %d \n" RESET,a->address,
           //   a->asic_temp*6+77, (temp_measure_temp1-1)*6+77);
            a->asic_temp = (ASIC_TEMP)(temp_measure_temp1-1);
         }
       }
      DBG(DBG_SCALING, "THERMAL UPDATED a[%x] to %d\n", a->address, a->asic_temp);
    }
    // enable back thermal shutdown
    enable_thermal_shutdown();
  }
}
#endif



