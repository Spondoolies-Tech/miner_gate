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

/*
       Supported temp sensor settings:
   temp    83 / 89 / 95 / 101 / 107 / 113 /119 / 125
   T[2:0]  0    1     2    3     4     5    6     7 
*/




uint32_t crc32(uint32_t crc, const void *buf, size_t size);
void print_state();
void print_scaling();

MINER_BOX vm = { 0 };
extern SPONDOOLIES_NVM nvm;
extern int assert_serial_failures;

int test_asic(int addr) {
  assert_serial_failures = false;
  int ver = read_reg_device(addr, ADDR_VERSION);
  assert_serial_failures = true;
  // psyslog("Testing loop, version = %x\n", ver);
  return ((ver & 0xFF) == 0x3c);
}



void pause_all_mining_engines() {
  int err;
  //passert(vm.asics_shut_down_powersave == 0);
  int some_asics_busy = read_reg_broadcast(ADDR_BR_CONDUCTOR_BUSY);
  set_fan_level(0);
  /*
  while(some_asics_busy != 0) {
    int addr = BROADCAST_READ_ADDR(some_asics_busy);
    psyslog(RED "some_asics_busy %x\n" RESET, some_asics_busy);
    disable_asic_forever_rt(addr);
    some_asics_busy = read_reg_broadcast(ADDR_BR_CONDUCTOR_BUSY);
  }
  // stop all ASICs
  disable_engines_all_asics();
  */
  // disable_engines_all_asics();
  vm.asics_shut_down_powersave = 1;
  psyslog("System has no jobs, going to sleep...\n");
}

void unpause_all_mining_engines() {
  int err;
  psyslog("Got mining request, enable DC2DC!\n");
  set_fan_level(vm.max_fan_level);  
  vm.not_mining_time = 0;
  
  //enable_good_engines_all_asics_ok();
  
  psyslog("Got mining request, waking up done!\n");
  vm.asics_shut_down_powersave = 0;
}


int test_serial(int loopid) {
  assert_serial_failures = false;
  int i = 0;
  int val1;
  int val2;
 

  while ((val1 = read_spi(ADDR_SQUID_STATUS)) &
         BIT_STATUS_SERIAL_Q_RX_NOT_EMPTY) {
    val2 = read_spi(ADDR_SQUID_SERIAL_READ);
    i++;
    if (i > 700) {
      // parse_squid_status(val1);
      // psyslog("ERR (%d) loop is too noisy: ADDR_SQUID_STATUS=%x,
      // ADDR_SQUID_SERIAL_READ=%x\n",i, val1, val2);
      psyslog("Loop NOISE\n");
      return 0;
    }
    usleep(10);
  }

  // psyslog("Ok %d!\n", i);

  // test for connectivity
  int ver = read_reg_broadcast(ADDR_VERSION);

  // printf("XTesting loop, version = %x\n", ver);
  if (BROADCAST_READ_DATA(ver) != 0x3c) {
    // printf("Failed: Got version: %x!\n", ver);
    psyslog("Loop BAD DATA (%x)\n",ver);
    return 0;
  }

  assert_serial_failures = true;
  return 1;
}

int enable_good_loops_ok() {
  write_spi(ADDR_SQUID_LOOP_BYPASS, 0xFFFFFF);
  write_spi(ADDR_SQUID_LOOP_RESET, 0);
  usleep(10000);
  write_spi(ADDR_SQUID_COMMAND, 0xF);
  write_spi(ADDR_SQUID_COMMAND, 0x0);
  write_spi(ADDR_SQUID_LOOP_BYPASS, ~(vm.good_loops));
  write_spi(ADDR_SQUID_LOOP_RESET, 0xFFFFFF); 
  int ok = test_serial(-1);
  if (!ok) {
    return 0;
  }
  /*
  for (int i = 0; i < LOOP_COUNT; i++) {
    psyslog("%sloop %d enabled = %d%s\n",
           (!vm.loop[i].enabled_loop) ? RED : RESET, i,
           (vm.loop[i].enabled_loop), RESET);
  }
  */
  return 1;
}






int count_ones(int failed_engines) {
  int c;
  for (c = 0; failed_engines; failed_engines >>= 1)
  {
    c += failed_engines & 1;
  }
  return c;
}



extern int rt_queue_size;
extern int rt_queue_sw_write;
extern int rt_queue_hw_done;

void print_adapter(FILE *f ) ;

void print_scaling() {
  int err;
  
  FILE *f = fopen("/var/log/asics", "w");
  if (!f) {
    psyslog("Failed to save ASIC state\n");
    return;
  }

  //hammer_iter_init(&hi);
  int total_hash_power=0;
  int theoretical_hash_power=0; 
  int wanted_hash_power=0;
  int total_loops=0;
  int total_asics=0;
  int expected_rate=0;
  fprintf(f, GREEN "\n----------\nAC2DC power=%d[%d], temp=%d, leading zeroes=%d\n" RESET,
      vm.ac2dc_power,
      vm.max_ac2dc_power,
      vm.ac2dc_temp,
      vm.cur_leading_zeroes
    );
  int total_watt=0;
  fprintf(f,GREEN  "L |Vtrm/max  |vlt|Wt|"  "Am|DCt|"  "crt|Gh|Ov" ); 
  for (int i = 0; i < HAMMERS_PER_LOOP/2; i++) {
    fprintf(f, "|ID |aTMP|Freq |Th|Bt|Engn|Wn ");
  }
  for (int addr = 0; addr < HAMMERS_COUNT ; addr++) {  
    hammer_iter hi;    
    hi.addr = addr;
    hi.a = &vm.hammer[addr];
    hi.l = addr/HAMMERS_PER_LOOP;
    hi.h = addr%HAMMERS_PER_LOOP;                                                                  
    if (hi.h == HAMMERS_PER_LOOP/2) {fprintf(f,GREEN RESET "\n                                     ");}
    if (hi.h == 0) {
      total_loops++;
      if (!vm.loop[hi.l].enabled_loop) {
        fprintf(f, GREEN RESET "\nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
      } else {    
        DC2DC* dc2dc = &vm.loop[hi.l].dc2dc;

        fprintf(f,  
          GREEN RESET "\n%2d|%4x(%4x)|%3d|%2d|"  
          "%s%2d%s|%s%3d%s|"   
          "%3d|%2d|%2d" , 
          hi.l, 
          vm.loop[hi.l].dc2dc.loop_vtrim&0xffff,
          vm.loop[hi.l].dc2dc.max_vtrim_currentwise&0xffff,
          VTRIM_TO_VOLTAGE_MILLI(vm.loop[hi.l].dc2dc.loop_vtrim),
          dc2dc->dc_power_watts_16s/16,
          
        ((dc2dc->dc_current_16s>=DC2DC_INITIAL_CURRENT_16S - 1*16)?RED:GREEN), dc2dc->dc_current_16s/16,GREEN,
        ((dc2dc->dc_temp>=DC2DC_TEMP_GREEN_LINE)?RED:GREEN), dc2dc->dc_temp,GREEN,
        
          vm.loop[hi.l].crit_temp_downscale,
          vm.loop[hi.l].asic_hz_sum*15/1000,
          vm.loop[hi.l].overheating_asics
          );

        total_watt+=dc2dc->dc_power_watts_16s;
      }
    }
    
    if (!hi.a->asic_present) {
      fprintf(f, GREEN RESET "|xxxxxxxxxxxxxxxxxxxxxxxxxxxxx");           
      continue;
    }

    total_hash_power += hi.a->freq_hw*15+210;  
    wanted_hash_power += hi.a->freq_wanted*15+210;
    theoretical_hash_power += hi.a->freq_thermal_limit*15+210;

    total_asics++;

    fprintf(f, GREEN RESET "|%3d:%s%3dc%s %s%3dhz%s(%2d/%2d)%s %x" GREEN RESET "%3d", 
      hi.addr,
      (hi.a->asic_temp>=vm.max_asic_temp-1)?((hi.a->asic_temp>=vm.max_asic_temp)?RED:YELLOW):GREEN,((hi.a->asic_temp*6)+77),GREEN,
       ((hi.a->freq_wanted>=ASIC_FREQ_540)? (MAGENTA) : ((hi.a->freq_wanted<=ASIC_FREQ_510)?(CYAN):(YELLOW))), hi.a->freq_wanted*15+210,GREEN,
       hi.a->freq_thermal_limit-hi.a->freq_wanted,
       hi.a->freq_bist_limit-hi.a->freq_wanted, 
       (vm.hammer[hi.addr].working_engines!=0x7FFF)?GREEN_BOLD:GREEN, vm.hammer[hi.addr].working_engines,
        vm.hammer[hi.addr].solved_jobs);
  }
  // print last loop
  // print total hash power
  fprintf(f, RESET "\n[H:HW:%dGh/Th:%dGh/W:%dGh,W:%d,L:%d,A:%d,ER:%d,EP:%d,MMtmp:%d]\n",
                (total_hash_power*15)/1000,
                theoretical_hash_power*15/1000,
                wanted_hash_power*15/1000,
                total_watt/16,
                total_loops,
                total_asics,
                (total_asics)?((total_hash_power*15*192)/1000/total_asics):0,
                (total_asics)?((vm.ac2dc_power-70)/total_asics*192+70):0,
                vm.mgmt_temp_max
  );

   fprintf(f, "Pushed %d jobs , in queue %d jobs!\n",
             vm.last_second_jobs, rt_queue_size);
   vm.last_second_jobs = 0;
    fprintf(f, "wins:%d[fp:%d], leading-zeroes:%d idle:%d/%d :)\n", vm.solved_jobs_total, vm.false_positives_total,
           vm.cur_leading_zeroes, vm.idle_probs, vm.busy_probs);
    print_adapter(f);
  fclose(f);
}
