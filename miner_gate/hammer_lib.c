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
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <netinet/in.h>
#include <unistd.h>
#include "mg_proto_parser.h"
#include "hammer.h"
#include "queue.h"
#include <string.h>
#include "ac2dc.h"
#include "hammer_lib.h"
#include "asic_thermal.h"
#include "board.h"

#include <pthread.h>
#include <spond_debug.h>
#include <sys/time.h>
#include "real_time_queue.h"
#include "miner_gate.h"
#include "scaling_manager.h"
#include <sched.h>
#include "pll.h"
#include "leds.h"


extern int kill_app;
extern pthread_mutex_t network_hw_mutex;
pthread_mutex_t hammer_mutex = PTHREAD_MUTEX_INITIALIZER;

int total_devices = 0;
extern int enable_reg_debug;

RT_JOB *add_to_sw_rt_queue(const RT_JOB *work);
void reset_sw_rt_queue();
RT_JOB *peak_rt_queue(uint8_t hw_id);
void push_job_to_hw_rt();

void dump_zabbix_stats();

void hammer_iter_init(hammer_iter *e) {
  e->addr = -1;
  e->done = 0;
  e->h = -1;
  e->l = 0;
  e->a = NULL;
}

static int pll_set_addr = 50;


int hammer_iter_next_present(hammer_iter *e) {
  int found = 0;
  
  while (!found) {
    //printf("l=%d,h=%d\n",e->l,e->h);
    passert(e->l < LOOP_COUNT);
    passert(e->h < HAMMERS_PER_LOOP);
    e->h++;

    if (e->h == HAMMERS_PER_LOOP) {
      e->l++;
      e->h = 0;
      while (!vm.loop[e->l].enabled_loop) {
        if (e->l == LOOP_COUNT) {
          //printf("return BEEE\n");
          e->done = 1;
          e->a = NULL;          
          return 0;
        }
        e->l++;
      }
    }

    if (e->l == LOOP_COUNT) {
      //printf("return Meee\n");
      e->done = 1;
      e->a = NULL;
      return 0;
    }

    if (vm.hammer[e->l * HAMMERS_PER_LOOP + e->h].asic_present) {
      found++;
    }
  }
  e->addr = e->l * HAMMERS_PER_LOOP + e->h;  
  e->a = vm.hammer + e->addr;
  return 1;
}

void loop_iter_init(loop_iter *e) {
  e->done = 0;
  e->l = -1;
}

int loop_iter_next_enabled(loop_iter *e) {
  int found = 0;

  while (!found) {
    e->l++;

    while (!vm.loop[e->l].enabled_loop) {
      e->l++;
    }

    if (e->l == LOOP_COUNT) {
      e->done = 1;
      return 0;
    }
  }
  return 1;
}

void stop_all_work_rt() {
  // wait to finish real time queue requests
#ifdef NO_PEAKS_STOP    
  write_reg_broadcast(ADDR_COMMAND, BIT_CMD_END_JOB_IF_Q_FULL);
  flush_spi_write();
  for (int i = 0; i < HAMMERS_PER_LOOP; i++ ) {
    for (int j = 0; j < LOOP_COUNT; j++ ) {
       HAMMER *h = &vm.hammer[i+j*HAMMERS_PER_LOOP];
       if (h->asic_present) {
         write_reg_device(i+j*HAMMERS_PER_LOOP,ADDR_COMMAND, BIT_CMD_END_JOB);
       }
    }
  } 
  flush_spi_write();  
  write_reg_broadcast(ADDR_COMMAND, BIT_CMD_END_JOB);  
  flush_spi_write();  
  vm.slow_asic_start = 1;
#else   
  write_reg_broadcast(ADDR_COMMAND, BIT_CMD_END_JOB);
  write_reg_broadcast(ADDR_COMMAND, BIT_CMD_END_JOB);
#endif  
  //usleep(100);
  RT_JOB work;

  // Move real-time q to complete.
  while (one_done_sw_rt_queue(&work)) {
    push_work_rsp(&work);
  }

  // Move pending to complete
  // RT_JOB w;
  /*
  while (pull_work_req(&work)) {
    push_work_rsp(&work);
  }*/
 // passert(read_reg_broadcast(ADDR_BR_CONDUCTOR_BUSY) == 0);
  vm.stopped_all_work = 1;
}

void resume_all_work() { 
  if (vm.stopped_all_work) {
    vm.stopped_all_work = 0;
  } 
}

void print_devreg(int reg, const char *name) {
  printf("%2x:  BR:%8x ", reg, read_reg_broadcast(reg));
  int h, l;

  hammer_iter hi;
  hammer_iter_init(&hi);

  while (hammer_iter_next_present(&hi)) {
    printf("DEV-%04x:%8x ", hi.a->address,
           read_reg_device(hi.addr, reg));
  }
  printf("  %s\n", name);
}

void parse_int_register(const char *label, int reg) {
  printf("%s :%x: ", label, reg);
  if (reg & BIT_INTR_NO_ADDR)
    printf("NO_ADDR ");
  if (reg & BIT_INTR_WIN)
    printf("WIN ");
  if (reg & BIT_INTR_FIFO_FULL)
    printf("FIFO_FULL ");
  if (reg & BIT_INTR_FIFO_ERR)
    printf("FIFO_ERR ");
  if (reg & BIT_INTR_CONDUCTOR_IDLE)
    printf("CONDUCTOR_IDLE ");
  if (reg & BIT_INTR_CONDUCTOR_BUSY)
    printf("CONDUCTOR_BUSY ");
  if (reg & BIT_INTR_RESERVED1)
    printf("RESERVED1 ");
  if (reg & BIT_INTR_FIFO_EMPTY)
    printf("FIFO_EMPTY ");
  if (reg & BIT_INTR_BIST_FAIL)
    printf("BIST_FAIL ");
  if (reg & BIT_INTR_THERMAL_SHUTDOWN)
    printf("THERMAL_SHUTDOWN ");
  if (reg & BIT_INTR_NOT_WIN)
    printf("NOT_WIN ");
  if (reg & BIT_INTR_0_OVER)
    printf("0_OVER ");
  if (reg & BIT_INTR_0_UNDER)
    printf("0_UNDER ");
  if (reg & BIT_INTR_1_OVER)
    printf("1_OVER ");
  if (reg & BIT_INTR_1_UNDER)
    printf("1_UNDER ");
  if (reg & BIT_INTR_PLL_NOT_READY)
    printf("PLL_NOT_READY ");
  printf("\n");
}

void print_state() {
  uint32_t i;
  static int print = 0;
  print++;
  dprintf("**************** DEBUG PRINT %x ************\n", print);
  print_devreg(ADDR_CURRENT_NONCE, "CURRENT NONCE");
  print_devreg(ADDR_BR_FIFO_FULL, "FIFO FULL");
  print_devreg(ADDR_BR_FIFO_EMPTY, "FIFO EMPTY");
  print_devreg(ADDR_BR_CONDUCTOR_BUSY, "BUSY");
  print_devreg(ADDR_BR_CONDUCTOR_IDLE, "IDLE");
  dprintf("*****************************\n", print);

  print_devreg(ADDR_CHIP_ADDR, "ADDR_CHIP_ADDR");
  print_devreg(ADDR_GOT_ADDR, "ADDR_GOT_ADDR ");
  print_devreg(ADDR_CONTROL, "ADDR_CONTROL ");
  print_devreg(ADDR_COMMAND, "ADDR_COMMAND ");
  print_devreg(ADDR_RESETING0, "ADDR_RESETING0");
  print_devreg(ADDR_RESETING1, "ADDR_RESETING1");
  print_devreg(ADDR_CONTROL_SET0, "ADDR_CONTROL_SET0");
  print_devreg(ADDR_CONTROL_SET1, "ADDR_CONTROL_SET1");
  print_devreg(ADDR_SW_OVERRIDE_PLL, "ADDR_SW_OVERRIDE_PLL");
  print_devreg(ADDR_PLL_RSTN, "ADDR_PLL_RSTN");
  print_devreg(ADDR_INTR_MASK, "ADDR_INTR_MASK");
  print_devreg(ADDR_INTR_CLEAR, "ADDR_INTR_CLEAR ");
  print_devreg(ADDR_INTR_RAW, "ADDR_INTR_RAW ");
  print_devreg(ADDR_INTR_SOURCE, "ADDR_INTR_SOURCE ");
  print_devreg(ADDR_BR_NO_ADDR, "ADDR_BR_NO_ADDR ");
  print_devreg(ADDR_BR_WIN, "ADDR_BR_WIN ");
  print_devreg(ADDR_BR_FIFO_FULL, "ADDR_BR_FIFO_FULL ");
  print_devreg(ADDR_BR_FIFO_ERROR, "ADDR_BR_FIFO_ERROR ");
  print_devreg(ADDR_BR_CONDUCTOR_IDLE, "ADDR_BR_CONDUCTOR_IDLE ");
  print_devreg(ADDR_BR_CONDUCTOR_BUSY, "ADDR_BR_CONDUCTOR_BUSY ");
  print_devreg(ADDR_BR_CRC_ERROR, "ADDR_BR_CRC_ERROR ");
  print_devreg(ADDR_BR_FIFO_EMPTY, "ADDR_BR_FIFO_EMPTY ");
  print_devreg(ADDR_BR_BIST_FAIL, "ADDR_BR_BIST_FAIL ");
  print_devreg(ADDR_BR_THERMAL_VIOLTION, "ADDR_BR_THERMAL_VIOLTION ");
  print_devreg(ADDR_BR_NOT_WIN, "ADDR_BR_NOT_WIN ");
  print_devreg(ADDR_BR_0_OVER, "ADDR_BR_0_OVER ");
  print_devreg(ADDR_BR_0_UNDER, "ADDR_BR_0_UNDER ");
  print_devreg(ADDR_BR_1_OVER, "ADDR_BR_1_OVER ");
  print_devreg(ADDR_BR_1_UNDER, "ADDR_BR_1_UNDER ");

  print_devreg(ADDR_MID_STATE, "ADDR_MID_STATE ");
  print_devreg(ADDR_MERKLE_ROOT, "ADDR_MERKLE_ROOT ");
  print_devreg(ADDR_TIMESTEMP, "ADDR_TIMESTEMP ");
  print_devreg(ADDR_DIFFICULTY, "ADDR_DIFFICULTY ");
  print_devreg(ADDR_WIN_LEADING_0, "ADDR_WIN_LEADING_0 ");
  print_devreg(ADDR_JOB_ID, "ADDR_JOB_ID");

  print_devreg(ADDR_WINNER_JOBID_WINNER_ENGINE,
               "ADDR_WINNER_JOBID_WINNER_ENGINE ");
  print_devreg(ADDR_WINNER_NONCE, "ADDR_WINNER_NONCE");
  print_devreg(ADDR_CURRENT_NONCE,
               CYAN "ADDR_CURRENT_NONCE" RESET);
  print_devreg(ADDR_LEADER_ENGINE, "ADDR_LEADER_ENGINE");
  print_devreg(ADDR_VERSION, "ADDR_VERSION");
  print_devreg(ADDR_ENGINES_PER_CHIP_BITS, "ADDR_ENGINES_PER_CHIP_BITS");
  print_devreg(ADDR_LOOP_LOG2, "ADDR_LOOP_LOG2");
  print_devreg(ADDR_ENABLE_ENGINE, "ADDR_ENABLE_ENGINE");
  print_devreg(ADDR_FIFO_OUT, "ADDR_FIFO_OUT");
  print_devreg(ADDR_CURRENT_NONCE_START, "ADDR_CURRENT_NONCE_START");
  print_devreg(ADDR_CURRENT_NONCE_START + 1, "ADDR_CURRENT_NONCE_START+1");
  print_devreg(ADDR_CURRENT_NONCE_START + 1, "ADDR_CURRENT_NONCE_START+2");

  print_devreg(ADDR_CURRENT_NONCE_RANGE, "ADDR_CURRENT_NONCE_RANGE");
  print_devreg(ADDR_FIFO_OUT_MID_STATE_LSB, "ADDR_FIFO_OUT_MID_STATE_LSB");
  print_devreg(ADDR_FIFO_OUT_MID_STATE_MSB, "ADDR_FIFO_OUT_MID_STATE_MSB");
  print_devreg(ADDR_FIFO_OUT_MARKEL, "ADDR_FIFO_OUT_MARKEL");
  print_devreg(ADDR_FIFO_OUT_TIMESTEMP, "ADDR_FIFO_OUT_TIMESTEMP");
  print_devreg(ADDR_FIFO_OUT_DIFFICULTY, "ADDR_FIFO_OUT_DIFFICULTY");
  print_devreg(ADDR_FIFO_OUT_WIN_LEADING_0, "ADDR_FIFO_OUT_WIN_LEADING_0");
  print_devreg(ADDR_FIFO_OUT_WIN_JOB_ID, "ADDR_FIFO_OUT_WIN_JOB_ID");
  print_devreg(ADDR_BIST_NONCE_START, "ADDR_BIST_NONCE_START");
  print_devreg(ADDR_BIST_NONCE_RANGE, "ADDR_BIST_NONCE_RANGE");
  print_devreg(ADDR_BIST_NONCE_EXPECTED, "ADDR_BIST_NONCE_EXPECTED");
  print_devreg(ADDR_EMULATE_BIST_ERROR, "ADDR_EMULATE_BIST_ERROR");
  print_devreg(ADDR_BIST_PASS, "ADDR_BIST_PASS");
  parse_int_register("ADDR_INTR_RAW", read_reg_broadcast(ADDR_INTR_RAW));
  parse_int_register("ADDR_INTR_MASK", read_reg_broadcast(ADDR_INTR_MASK));
  parse_int_register("ADDR_INTR_SOURCE", read_reg_broadcast(ADDR_INTR_SOURCE));

  // print_engines();
}

// Queues work to actual HW
// returns 1 if found nonce
void push_to_hw_queue_rt(RT_JOB *work) {
  //flush_spi_write();
  static int j = 0;
  int i;
  // printf("Push!\n");
  //
  write_reg_broadcast(ADDR_COMMAND, BIT_CMD_END_JOB_IF_Q_FULL);

  for (i = 0; i < 8; i++) {
    write_reg_broadcast((ADDR_MID_STATE + i), work->midstate[i]);
  }
  write_reg_broadcast(ADDR_MERKLE_ROOT, work->mrkle_root);
  write_reg_broadcast(ADDR_TIMESTEMP, work->timestamp);
  write_reg_broadcast(ADDR_DIFFICULTY, work->difficulty);
  /*
  if (current_leading == 0) {
   push_write_reg_broadcast(ADDR_WIN_LEADING_0, work->leading_zeroes);
   current_leading = work->leading_zeroes;
  }
  */
  //passert(work->work_id_in_hw < 0x100);
  write_reg_broadcast(ADDR_JOB_ID, work->work_id_in_hw);
  //write_reg_broadcast(ADDR_COMMAND, BIT_CMD_END_JOB_IF_Q_FULL);
#ifdef NO_PEAKS_START    
  if (vm.slow_asic_start == 1) {
    for (int i = 0; i < HAMMERS_PER_LOOP; i++ ) {
       for (int j = 0; j < LOOP_COUNT; j++ ) {
          HAMMER *h = &vm.hammer[i+j*HAMMERS_PER_LOOP];
          if (h->asic_present) {
            write_reg_device(i+j*HAMMERS_PER_LOOP,ADDR_COMMAND, BIT_CMD_LOAD_JOB);
          }
       }
    }
    flush_spi_write();
    vm.slow_asic_start = 0;
  } else {
    write_reg_broadcast(ADDR_COMMAND, BIT_CMD_LOAD_JOB);
  }
#else
  write_reg_broadcast(ADDR_COMMAND, BIT_CMD_LOAD_JOB);
#endif
  flush_spi_write();
}



// destribute range between 0 and max_range
void set_nonce_range_in_engines(unsigned int max_range) {
  int d, e;
  unsigned int current_nonce = 0;
  unsigned int engine_size;
  int total_engines = ENGINES_PER_ASIC * HAMMERS_COUNT;
  engine_size = ((max_range) / (total_engines));

  // for (d = FIRST_CHIP_ADDR; d < FIRST_CHIP_ADDR+total_devices; d++) {

  // int h, l;
  hammer_iter hi;
  hammer_iter_init(&hi);

  while (hammer_iter_next_present(&hi)) {
    write_reg_device(hi.addr, ADDR_CURRENT_NONCE_RANGE, engine_size);
    for (e = 0; e < ENGINES_PER_ASIC; e++) {
      //DBG(DBG_HW, "engine %x:%x got range from 0x%x to 0x%x\n", hi.addr, e,
       //   current_nonce, current_nonce + engine_size);
      write_reg_device(hi.addr, ADDR_CURRENT_NONCE_START + e, current_nonce);
      // read_reg_device(d, ADDR_CURRENT_NONCE_START + e);
      current_nonce += engine_size;
    }
  } 
  flush_spi_write();
  // print_state();
}


int allocate_addresses_to_devices_on_loop(int l, int override_vm) {
     // Only in loops discovered in "enable_good_loops_ok()"
       // Disable all other loops
       unsigned int bypass_loops = (~(1 << l) & 0xFFFFFF);
       printf("Giving address on loop (mask): %x\n", bypass_loops);
       write_spi(ADDR_SQUID_LOOP_BYPASS, bypass_loops);
       
       // Give 8 addresses
       int h = 0;
       int asics_in_loop = 0;
       for (h = 0; h < HAMMERS_PER_LOOP; h++) {
         int addr = l * HAMMERS_PER_LOOP + h;
         vm.hammer[addr].address = addr;
         vm.hammer[addr].loop_address = l;
         vm.loop[l].asic_count++;
         if (read_reg_broadcast(ADDR_BR_NO_ADDR)) {
           write_reg_broadcast(ADDR_CHIP_ADDR, addr);
           asics_in_loop++;
           vm.hammer[addr].asic_present = 1;           
           if (override_vm) {
             total_devices++;
             vm.loop[l].asic_count--;
             vm.hammer[addr].working_engines = ALL_ENGINES_BITMASK;  
             vm.hammer[addr].passed_last_bist_engines = ALL_ENGINES_BITMASK;
             vm.hammer[addr].freq_thermal_limit = MAX_ASIC_FREQ - 1;
             vm.hammer[addr].freq_bist_limit = MAX_ASIC_FREQ - 1;
             vm.hammer[addr].freq_wanted = MINIMAL_ASIC_FREQ;
           }
         } else {
           vm.hammer[addr].address = addr;
           vm.hammer[addr].loop_address = l;
           vm.hammer[addr].asic_present = 0;
           vm.hammer[addr].working_engines = 0;
         }
       }
       // Dont remove this print - used by scripts!!!!
       printf("%sASICS in loop %d: %d%s\n",
              (asics_in_loop == 8) ? RESET : RED, l,
              asics_in_loop, RESET);
       passert(read_reg_broadcast(ADDR_BR_NO_ADDR) == 0);

}


int allocate_addresses_to_devices() {
  int reg;
  int l;
  total_devices = 0;

  // Give resets to all ASICs
  DBG(DBG_HW, "Unallocate all chip addresses\n");
  write_reg_broadcast(ADDR_GOT_ADDR, 0);

  //  validate address reset worked
  reg = read_reg_broadcast(ADDR_BR_NO_ADDR);
  if (reg == 0) {
    passert(0);
  }

  for (l = 0; l < LOOP_COUNT; l++) {
    if (vm.loop[l].enabled_loop) {
      allocate_addresses_to_devices_on_loop(l, 1);
    } else {
      psyslog(RED "ASICS in loop %d: 0\n" RESET, l);
    }
  }
  // printf("3\n");
  write_spi(ADDR_SQUID_LOOP_BYPASS, (~(vm.good_loops))&0xFFFFFF);

  // Validate all got address
  passert(read_reg_broadcast(ADDR_BR_NO_ADDR) == 0);
  //  passert(read_reg_broadcast(ADDR_VERSION) != 0);
  DBG(DBG_HW, "Number of ASICs found: %x (LOOPS=%X) %X\n", 
  total_devices,(~(vm.good_loops))&0xFFFFFF,read_reg_broadcast(ADDR_VERSION));
  return total_devices;

  // set_bits_offset(i);
}

void memprint(const void *m, size_t n) {
  int i;
  for (i = 0; i < n; i++) {
    printf("%02x", ((const unsigned char *)m)[i]);
  }
}



int SWAP32(int x);
void compute_hash(const unsigned char *midstate, unsigned int mrkle_root,
                  unsigned int timestamp, unsigned int difficulty,
                  unsigned int winner_nonce, unsigned char *hash);
int get_leading_zeroes(const unsigned char *hash);

int get_print_win(int winner_device) {
  // TODO - find out who one!
  // int winner_device = winner_reg >> 16;
  // enable_reg_debug = 1;
  uint32_t winner_nonce; // = read_reg_device(winner_device, ADDR_WINNER_NONCE);
  uint32_t winner_id;    // = read_reg_device(winner_device, ADDR_WINNER_JOBID);
  int engine_id;
  uint32_t next_win_reg = 0; 
  write_reg_device(winner_device, ADDR_INTR_CLEAR, BIT_INTR_WIN);  
  push_hammer_read(winner_device, ADDR_WINNER_NONCE, &winner_nonce);
  push_hammer_read(winner_device, ADDR_WINNER_JOBID_WINNER_ENGINE, &winner_id);
  push_hammer_read(BROADCAST_ADDR, ADDR_BR_WIN, &next_win_reg);
  squid_wait_hammer_reads();
  // Winner ID holds ID and Engine info
  engine_id = winner_id >> 8;
  winner_id = winner_id & 0xFF;
  RT_JOB *work_in_hw = peak_rt_queue(winner_id);

  if ((winner_nonce != 0) &&
      (work_in_hw->work_state == WORK_STATE_HAS_JOB) &&
      ((work_in_hw->winner_nonce[0] == 0) || (work_in_hw->winner_nonce[1] == 0))) {
#if 0  
    memcpy((const unsigned char*)vm.last_win.midstate, (const unsigned char*)work_in_hw->midstate, sizeof(work_in_hw->midstate));   
    vm.last_win.mrkle_root = work_in_hw->mrkle_root;
    vm.last_win.timestamp = work_in_hw->timestamp;
    vm.last_win.difficulty = work_in_hw->difficulty;
    vm.last_win.winner_nonce = winner_nonce;    
    vm.last_win.winner_engine = engine_id;    
    vm.last_win.winner_device = winner_device;
#endif
  
#if 0   
   static unsigned char hash[32];
   //memset(hash,0, 32);
   compute_hash((const unsigned char*)work_in_hw->midstate,
       SWAP32(work_in_hw->mrkle_root),
       SWAP32(work_in_hw->timestamp),
       SWAP32(work_in_hw->difficulty),
       SWAP32(winner_nonce),
       //0x7c2bac1d,
       hash);
   //memprint((void*)hash, 32);
   int leading_zeroes = get_leading_zeroes(hash);
   DBG(DBG_WINS,"Win leading Zeroes: %d\n", leading_zeroes);
   if (leading_zeroes < 30) {
      psyslog("FP on %d loop %d (Lz=%x)\n", winner_device, winner_device/HAMMERS_PER_LOOP, leading_zeroes);
      vm.false_positives_total++;
      // delete win and down ASIC
      winner_nonce = 0;
      //vm.hammer[winner_device].passed_last_bist_engines &= 0x7EFE;
   } else if (leading_zeroes < vm.cur_leading_zeroes) {
      // not real win.
      //printf("Fake win\n");
      winner_nonce = 0;
   } else {
      //work_in_hw->winner_nonce = winner_nonce;
      //printf("Win %d\n", winner_id);
   }
   //end_stopper(&tv, "Win compute");
#endif      
    if (work_in_hw->winner_nonce[0] == 0) {
      work_in_hw->winner_nonce[0] = winner_nonce;
	  if (work_in_hw->ntime_offset) {
         work_in_hw->timestamp = ntohl(ntohl(work_in_hw->timestamp) - work_in_hw->ntime_offset);  
      }
    } else {
      work_in_hw->winner_nonce[1] = winner_nonce;
    }
    
    
    // Push wins imediatly
    //push_work_rsp(work_in_hw);
    vm.concecutive_bad_wins = 0;
  } else {
    psyslog( "!!!!!  Warning !!!!: Win orphan job 0x%x or double win, nonce1=0x%x  , nonce=0x%x!!!\n" ,
      winner_id,  
      work_in_hw->winner_nonce[1],
      winner_nonce);
    vm.concecutive_bad_wins++;
    if (vm.concecutive_bad_wins > 300) {
      // Hammers out of sync.
      static char x[200]; 
      sprintf(x, "concecutive_bad_wins");
      mg_event(x);
      exit_nicely();
    }
  }
  return next_win_reg;
}

/*
void fill_random_work(RT_JOB *work) {
 static int id = 1;
 id++;
 memset(work, 0, sizeof(RT_JOB));
 work->midstate[0] = (rand()<<16) + rand();
 work->midstate[1] = (rand()<<16) + rand();
 work->midstate[2] = (rand()<<16) + rand();
 work->midstate[3] = (rand()<<16) + rand();
 work->midstate[4] = (rand()<<16) + rand();
 work->midstate[5] = (rand()<<16) + rand();
 work->midstate[6] = (rand()<<16) + rand();
 work->midstate[7] = (rand()<<16) + rand();
 work->work_id_in_sw = id;
 work->mrkle_root = (rand()<<16) + rand();
 work->timestamp  = (rand()<<16) + rand();
 work->difficulty = (rand()<<16) + rand();
 //work_in_queue->winner_nonce
}*/

int init_hammers() {
  int i;
  // enable_reg_debug = 1;
  psyslog("VERSION SW:%x\n", 1);
  // FOR FPGA ONLY!!! TODO
  /*
  printf(RED "FPGA TODO REMOVE!\n" RESET);
  write_reg_broadcast(ADDR_LEADER_ENGINE, 0x0);
  write_reg_broadcast(ADDR_SW_OVERRIDE_PLL, 0x1);
  write_reg_broadcast(ADDR_PLL_RSTN, 0x1);
  */
  
  // Clearing all interrupts
  write_reg_broadcast(ADDR_INTR_MASK, 0x0);
  write_reg_broadcast(ADDR_INTR_CLEAR, 0xffff);
  write_reg_broadcast(ADDR_INTR_CLEAR, 0x0);


  flush_spi_write();
  // print_state();
  return 0;
}


typedef struct {
  uint32_t nonce_winner;
  uint32_t midstate[8];
  uint32_t mrkl;
  uint32_t timestamp;
  uint32_t difficulty;
  uint32_t leading;
} BIST_VECTOR;


#define TOTAL_BISTS 8
BIST_VECTOR bist_tests[TOTAL_BISTS] = 
{
{0x1DAC2B7C,
  {0xBC909A33,0x6358BFF0,0x90CCAC7D,0x1E59CAA8,0xC3C8D8E9,0x4F0103C8,0x96B18736,0x4719F91B},
    0x4B1E5E4A,0x29AB5F49,0xFFFF001D,0x26},

{0x227D01DA,
  {0xD83B1B22,0xD5CEFEB4,0x17C68B5C,0x4EB2B911,0x3C3D668F,0x5CCA92CA,0x80E63EEC,0x77415443},
    0x31863FE8,0x68538051,0x3DAA011A,0x20},
    
{0x481df739,
  {0x4a41f9ac,0x30309e3f,0x06d49e05,0x938f5ceb,0x90ef75e3,0x94fb77e4,0x9e851664,0x3a89aa95},
  0x73ae0eb3, 0xb8d6fb46, 0xf72e17e4, 36},


{0xbf9d8004,
  { 0x55a41303,0xeea9d4d3,0x39aa38f4,0x985a231f,0xbea0d01f,0x23218acc,0x08c65f3f,0x9535c3f7},
  0x86fb069c, 0xb0a01a68, 0xaf7f89b0, 36},


{0xbbff18c5,
  {0x02c397d8,0xececa9bc,0xc31e67f0,0x9a6c1b50,0x684206bc,0xd09e1e48,0x32a30c61,0x4f7f9717},
0xb9a63c7e,0x0fa4fe30,0x7ebf0578,34},



{0x331f38d8,
  { 0x72608c6a,0xb1dae622,0x74e0fd9d,0x80af02b4,0x0d3c4f66,0x3a0868d6,0x30d6d1cd,0x54b0e059},
  0x4dc7d05e, 0x45716f49, 0x25b433d9, 36},

{0x339bc585,
  { 0xa40ebbec,0x6aa3645e,0xc0ef8027,0xb83c0010,0x0eeda850,0xbc407eae,0x0d892b47,0x6b1f7541},
  0xd6b747c9, 0xd12764f2, 0xd026d972, 35},

{0xbbed44a1 ,
  {   0x0dd441c1,0x5802c0da,0xf2951ee4,0xd77909da,0xb25c02cb,0x009b6349,0xfe2d9807,0x257609a8 },
  0x5064218c, 0x57332e1b, 0xcef3abae, 35}
};




// returns 1 on failed
int do_bist_ok_rt(int long_bist) {
  // Choose random bist.
  static int bist_id = 0;
  int poll_counter = 0;
  int failed = 0;
     
  // Enter BIST mode
#ifdef NO_PEAKS_STOP
  write_reg_broadcast(ADDR_COMMAND, BIT_CMD_END_JOB_IF_Q_FULL);
  flush_spi_write();
  // write_reg_broadcast(ADDR_COMMAND, BIT_CMD_END_JOB);
  for (int i = 0; i < HAMMERS_PER_LOOP; i++ ) {
    for (int j = 0; j < LOOP_COUNT; j++ ) {
       HAMMER *h = &vm.hammer[i+j*HAMMERS_PER_LOOP];
       if (h->asic_present) {
         write_reg_device(i+j*HAMMERS_PER_LOOP,ADDR_COMMAND, BIT_CMD_END_JOB);
       }
    }
  } 
  flush_spi_write();  
  write_reg_broadcast(ADDR_COMMAND, BIT_CMD_END_JOB);  
  vm.slow_asic_start = 1;
#else
  write_reg_broadcast(ADDR_COMMAND, BIT_CMD_END_JOB);
  write_reg_broadcast(ADDR_COMMAND, BIT_CMD_END_JOB);
#endif
  write_reg_broadcast(ADDR_CONTROL_SET1, BIT_CTRL_BIST_MODE);  
  flush_spi_write();

 

   // Give BIST jobc
   write_reg_broadcast(ADDR_BIST_NONCE_START, bist_tests[bist_id].nonce_winner - 10000+(rand()%10)); // Randomise test a bit
   write_reg_broadcast(ADDR_BIST_NONCE_RANGE, 10500);
   write_reg_broadcast(ADDR_BIST_NONCE_EXPECTED, bist_tests[bist_id].nonce_winner); 
   write_reg_broadcast(ADDR_MID_STATE + 0, bist_tests[bist_id].midstate[0]);
   write_reg_broadcast(ADDR_MID_STATE + 1, bist_tests[bist_id].midstate[1]);
   write_reg_broadcast(ADDR_MID_STATE + 2, bist_tests[bist_id].midstate[2]);
   write_reg_broadcast(ADDR_MID_STATE + 3, bist_tests[bist_id].midstate[3]);
   write_reg_broadcast(ADDR_MID_STATE + 4, bist_tests[bist_id].midstate[4]);
   write_reg_broadcast(ADDR_MID_STATE + 5, bist_tests[bist_id].midstate[5]);
   write_reg_broadcast(ADDR_MID_STATE + 6, bist_tests[bist_id].midstate[6]);
   write_reg_broadcast(ADDR_MID_STATE + 7, bist_tests[bist_id].midstate[7]);
   write_reg_broadcast(ADDR_MERKLE_ROOT, bist_tests[bist_id].mrkl);
   write_reg_broadcast(ADDR_TIMESTEMP, bist_tests[bist_id].timestamp);
   write_reg_broadcast(ADDR_DIFFICULTY, bist_tests[bist_id].difficulty);
   write_reg_broadcast(ADDR_WIN_LEADING_0, bist_tests[bist_id].leading);
   write_reg_broadcast(ADDR_JOB_ID, 0xEE);
   //write_reg_broadcast(ADDR_COMMAND, BIT_CMD_LOAD_JOB);
   flush_spi_write();
   bist_id = (bist_id+1)%TOTAL_BISTS;


   for (int i = 0; i < HAMMERS_PER_LOOP; i++ ) {
     for (int j = 0; j < LOOP_COUNT; j++ ) {
        HAMMER *h = &vm.hammer[i+j*HAMMERS_PER_LOOP];
        if (h->asic_present) {
          write_reg_device(i+j*HAMMERS_PER_LOOP,ADDR_COMMAND, BIT_CMD_LOAD_JOB);
        }
     }
     flush_spi_write();
   } 
  
  
  
   int i = 0;
   int res;
   while ((res = read_reg_broadcast(ADDR_BR_CONDUCTOR_BUSY))) {
     if (i == 100) {
         int addr = BROADCAST_READ_ADDR(res);
         psyslog("Asic %x didnt finish BIST\n", addr);
       break;
     }
     i++;
   }


  int bist_fail;
  while (bist_fail = read_reg_broadcast(ADDR_BR_BIST_FAIL)) {
    uint16_t failed_addr = BROADCAST_READ_ADDR(bist_fail);
    HAMMER *h = ASIC_GET_BY_ADDR(failed_addr);
    h->passed_last_bist_engines = read_reg_device(failed_addr, ADDR_BIST_PASS);
    
   /* printf(RED "Failed Bist[%d] %x::%x \n" RESET, j, failed_addr, 
                h->passed_last_bist_engines, h->freq_wanted, h->freq_hw);*/
    write_reg_device(failed_addr, ADDR_INTR_CLEAR, BIT_INTR_BIST_FAIL);
    write_reg_device(failed_addr, ADDR_INTR_CLEAR, BIT_INTR_WIN);
    write_reg_device(failed_addr, ADDR_CONTROL_SET0, BIT_CTRL_BIST_MODE);
    flush_spi_write();
    // print_devreg(ADDR_INTR_RAW , "ADDR_INTR_RAW ");
    failed++;
    //passert(0);
  }

  write_reg_broadcast(ADDR_INTR_CLEAR, BIT_INTR_WIN);
  //printf("polls=%d, bist_id=%d\n", i,bist_id);
 
  uint32_t single_win_test;
#if 0  
  if (1) {
    push_hammer_read(hi.addr, ADDR_BR_WIN, &single_win_test);
    //read_reg_device(hi.addr, ADDR_BR_WIN);
  }
#endif  
  // Exit BIST
  //passert(read_reg_broadcast(ADDR_BR_WIN));
  
  write_reg_broadcast(ADDR_CONTROL_SET0, BIT_CTRL_BIST_MODE);
  write_reg_broadcast(ADDR_WIN_LEADING_0, vm.cur_leading_zeroes);
  flush_spi_write();
  return failed;
}

int pull_work_req(RT_JOB *w);
void print_adapter();
int has_work_req();

void one_minute_tasks() {
  static int cnt = 0;
  cnt++;
  // Give them chance to raise over 3 hours if system got colder
  psyslog("Last minute rate: %d (m:%d, nm:%d)\n", (vm.solved_difficulty_total*4/60), vm.mining_time, vm.not_mining_time)
  vm.solved_difficulty_total = 0;
  if (cnt%(60*3) == 0) {
    // Lets forget all scaling once every day!
    psyslog("Forget SCALING!");
    for (int i = 0 ; i < LOOP_COUNT; i++) {
      if (vm.loop[i].enabled_loop) {
        vm.loop[i].dc2dc.max_vtrim_currentwise = vm.vtrim_max;
      }
    }
  }
}


void ten_second_tasks() { 
 
}
/*
pthread_mutex_t i2c_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_lock(&i2c_mutex);
*/


void once_second_tasks_rt() {
  struct timeval tv;
  static int counter = 0;
  //start_stopper(&tv);
  
  if (vm.cosecutive_jobs >= MIN_COSECUTIVE_JOBS_FOR_SCALING) {
      if ((vm.start_mine_time == 0)) {
        vm.start_mine_time = time(NULL);
        set_light(LIGHT_GREEN, LIGHT_MODE_ON);
        psyslog( "Restarting mining timer\n" );
      }
  }
  
  for (int i=0; i< HAMMERS_COUNT; i++) {
    HAMMER *h = &vm.hammer[i];
    if (!h->asic_present) {
      continue;
    }
        
    if (h->too_hot_temp_counter > TOO_HOT_COUNER_DISABLE_ASIC) {
      psyslog("Disabling HOT asic %d\n", h->address);
      disable_asic_forever_rt(i);
    }
  }


  if (vm.cosecutive_jobs == 0) {
    vm.start_mine_time = 0;
    set_light(LIGHT_GREEN, LIGHT_MODE_FAST_BLINK);
    vm.not_mining_time++;
    vm.mining_time = 0;
  } else {
    vm.not_mining_time = 0;
    vm.mining_time++;
  }
  // See if we can stop engines
  //psyslog("not_mining_time %d\n", vm.not_mining_time);
  
  if (vm.not_mining_time >= IDLE_TIME_TO_PAUSE_ENGINES) {
    if (!vm.asics_shut_down_powersave) {
      pause_all_mining_engines();
    }
  } 

  if (!vm.asics_shut_down_powersave) {
    // Change frequencies if needed
    if (vm.cosecutive_jobs >= MIN_COSECUTIVE_JOBS_FOR_SCALING) {
      do_bist_fix_loops_rt();
    }

  } 

  if (counter % 10 == 0) {
    ten_second_tasks(); 
  }

  
  if (counter % 60 == 2) {
    one_minute_tasks();
  }
  ++counter;
  
 // end_stopper(&tv,"1SEC_RT");
}


// 40 times per second
// RUN FROM DIFFERENT THREAD
int loop_can_down(int l);
void loop_down(int l);
void asic_down_completly(HAMMER *a);

int update_vm_with_currents_and_temperatures_nrt() {
  static int counter = 0;
  counter++;
  int err;
  int critical_current;

  // 38 times a second, poll 1 dc2dc ~2 times a second.
  static int loop = 0;

  // Update DC2DC
  if (vm.loop[loop].enabled_loop) {
    int overcurrent = 0, oc_warning=0;
    update_dc2dc_current_temp_measurments(loop, &overcurrent, &oc_warning); 
    critical_current = (vm.loop[loop].dc2dc.dc_current_16s > vm.loop[loop].dc2dc.dc_current_limit_16s);
    if (critical_current || oc_warning) {

        if (oc_warning) {
          vm.loop[loop].down_scale_type += (0x10); 
        }

        if (critical_current) {
          vm.loop[loop].down_scale_type += (0x1); 
        }
        psyslog(YELLOW "CURRENT warrning detected %d on loop %d\n" RESET, vm.loop[loop].dc2dc.dc_current_16s ,loop);  
        if (time(NULL) - vm.loop[loop].dc2dc.last_downscale_time < 2) {
          psyslog(YELLOW "No downscale, too fast\n" RESET);
        } else {
          int l = loop;
          if (loop_can_down(l)) {
           if (vm.loop[l].dc2dc.loop_vtrim > VTRIM_MIN) {       
              vm.loop[l].dc2dc.max_vtrim_currentwise = vm.loop[l].dc2dc.loop_vtrim-1;
              loop_down(l);
              vm.loop[l].dc2dc.last_downscale_time = time(NULL);
              vm.needs_scaling = 1;      

              for (int h = l*HAMMERS_PER_LOOP; h < l*HAMMERS_PER_LOOP+HAMMERS_PER_LOOP;h++) {
                if (vm.hammer[h].asic_present) {
                    asic_down_completly(&vm.hammer[h]);
                    break;
                }
              }
           }
          }
      }
     
    }

    if (overcurrent) {
      struct timeval tv;
      start_stopper(&tv);
      psyslog(RED "DC2DC ERROR STAGE0, disabling loop! %d\n" RESET, loop);
      pthread_mutex_lock(&hammer_mutex);
#ifdef FIX_OC_ERRORS
      if (vm.loop[loop].dc2dc.loop_vtrim > VTRIM_MIN) {       
         vm.loop[loop].dc2dc.max_vtrim_currentwise = vm.loop[loop].dc2dc.loop_vtrim-1;
      }
      static char x[200]; 
      sprintf(x, "Loop %x down", loop);
      mg_event(x);
      // close all loops accept l and give reset
      unsigned int bypass_loops = (~(1 << loop) & 0xFFFFFF);
      write_spi(ADDR_SQUID_LOOP_BYPASS, bypass_loops);
      printf(YELLOW "CLOSED ALL LOOPS BUT %d\n" RESET, loop);
      write_spi(ADDR_SQUID_LOOP_RESET, bypass_loops);
      usleep(1000);
      write_spi(ADDR_SQUID_LOOP_BYPASS, vm.good_loops);
      write_spi(ADDR_SQUID_LOOP_RESET, vm.good_loops);
      write_spi(ADDR_SQUID_LOOP_BYPASS, bypass_loops);
      
      init_hammers();

      int addra;
      while (addra = BROADCAST_READ_ADDR(read_reg_broadcast(ADDR_BR_CONDUCTOR_BUSY))) {
           psyslog(RED "CONDUCTOR BUZY IN %x (%X)\n" RESET, addra,read_reg_broadcast(ADDR_VERSION));
      }

      
      addra = test_serial(loop);
      write_reg_broadcast(ADDR_GOT_ADDR, 0);
      //  validate address reset worked
      int reg = read_reg_broadcast(ADDR_BR_NO_ADDR);
      printf(YELLOW "!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
        "LOOP %d OK:::%d, vm.good_loops=%x, bypass:%x no_addr:%x\n" RESET, 
        loop, addra, vm.good_loops, bypass_loops, reg);      


      // give addresses
      allocate_addresses_to_devices_on_loop(loop, 0);

      for (int h = 0; h < HAMMERS_PER_LOOP; h++) {
               int addr = loop * HAMMERS_PER_LOOP + h;
               enable_engines_asic(addr, vm.hammer[h].working_engines);
               vm.hammer[h].freq_hw = (ASIC_FREQ)(vm.hammer[h].freq_wanted - 1);
      }
      //enable_reg_debug = 0;
      
      set_nonce_range_in_engines(0xFFFFFFFF);
      thermal_init();
      for (int h = 0; h < HAMMERS_PER_LOOP; h++) {
         int addr = loop * HAMMERS_PER_LOOP + h;
         enable_engines_asic(addr, vm.hammer[h].working_engines);
         vm.hammer[h].freq_hw = (ASIC_FREQ)(vm.hammer[h].freq_wanted - 1);
      }
      write_reg_broadcast(ADDR_WIN_LEADING_0, vm.cur_leading_zeroes);
      write_spi(ADDR_SQUID_LOOP_BYPASS, (~(vm.good_loops))&0xFFFFFF);
#else   
      error are you sure?
      for (int i = loop*HAMMERS_PER_LOOP; i < loop*HAMMERS_PER_LOOP + HAMMERS_PER_LOOP ; i++) {
        if (vm.hammer[i].asic_present) {
          psyslog(RED "Disabling bad DC asic %d\n" RESET, i);
          disable_asic_forever_rt(i);
        }
      }
      vm.good_loops = vm.good_loops & ~(1<<loop);
      printf("Setting good loops to %x\n", vm.good_loops);
      write_spi(ADDR_SQUID_LOOP_BYPASS, (~(vm.good_loops))&0xFFFFFF);
      vm.loop[loop].enabled_loop = 0;
      vm.overcurrent_loops++;
      dc2dc_disable_dc2dc(loop, &err);
      if (vm.overcurrent_loops > 3) {
        psyslog("TOO MANY OC LOOPS, EXITING\n");
        exit_nicely(5);
      }      
#endif 
      end_stopper(&tv,"Fix loops");
      pthread_mutex_unlock(&hammer_mutex);
    }
  }

  // Wait for asics to shut
  loop = (loop+1)%LOOP_COUNT;
  return critical_current;
}




#ifndef ALL_TEMP_MONITOR
void set_temp_reading_rt(int measure_temp_addr, uint32_t* intr) {
    //flush_spi_write();
    write_reg_device(measure_temp_addr, ADDR_COMMAND, BIT_CMD_TS_RESET_1 | BIT_CMD_TS_RESET_0);
    push_hammer_read(measure_temp_addr, ADDR_INTR_RAW, intr);
}

void proccess_temp_reading_rt(HAMMER *a, int intr) {
     if (!(intr & BIT_INTR_0_OVER)) {
       if (a->asic_temp > (vm.max_asic_temp-2)) {
           a->asic_temp = (ASIC_TEMP)(vm.max_asic_temp-2);
           a->too_hot_temp_counter=0;
       }
     } else if ((intr & BIT_INTR_1_OVER)) { 
        if ((a->asic_temp < vm.max_asic_temp)) {
          a->asic_temp = (ASIC_TEMP)(vm.max_asic_temp);
          a->too_hot_temp_counter++;
        }
     } else {
          a->asic_temp = (ASIC_TEMP)(vm.max_asic_temp-1);          
          a->too_hot_temp_counter=0;
     }
}
#else
int current_reading = ASIC_TEMP_125;
void set_temp_reading_rt(int measure_temp_addr, uint32_t* intr) {
    //flush_spi_write();
    if (measure_temp_addr == 0) {
      current_reading = current_reading + 2;
      if (current_reading >= ASIC_TEMP_COUNT) {
         current_reading = ASIC_TEMP_89;
      }
    }
    write_reg_device(measure_temp_addr, ADDR_COMMAND, BIT_CMD_TS_RESET_1 | BIT_CMD_TS_RESET_0);
    write_reg_broadcast(ADDR_TS_SET_0, (current_reading-2));
    // Set to vm.max_asic_temp
    write_reg_broadcast(ADDR_TS_SET_1, (current_reading-1));
    write_reg_device(measure_temp_addr, ADDR_COMMAND, BIT_CMD_TS_RESET_1 | BIT_CMD_TS_RESET_0);
    push_hammer_read(measure_temp_addr, ADDR_INTR_RAW, intr);
}

void proccess_temp_reading_rt(HAMMER *a, int intr) {
     if (!(intr & BIT_INTR_0_OVER)) {
       if (a->asic_temp > (current_reading-2)) {
           a->asic_temp = (ASIC_TEMP)(current_reading-2);
           //a->too_hot_temp_counter=0;
       }
     } else if ((intr & BIT_INTR_1_OVER)) { 
        if ((a->asic_temp < current_reading)) {
          a->asic_temp = (ASIC_TEMP)(current_reading);
          //a->too_hot_temp_counter++;
        }
     } else {
          a->asic_temp = (ASIC_TEMP)(current_reading-1);          
          //a->too_hot_temp_counter=0;
     }
}

#endif
// 30 times a second - poll win and 1 temperature 
// and set 1 pll
#if 0
void once_33_msec_pll_rt() {
       struct timeval tv;
       if (vm.pll_changed != 0) {
         printf("PLL change start counter=%d\n",0);
         stop_all_work_rt();
         start_stopper(&tv);
         disable_engines_all_asics();
         printf("PLL change middle\n");
         // Start PLL
         for (int i = 0 ; i < HAMMERS_COUNT; i++) {
           if (vm.hammer[i].asic_present &&
               (vm.hammer[i].freq_wanted != vm.hammer[i].freq_hw)) {
              printf("PLL change %x\n",i);
              set_pll(pll_set_addr, vm.hammer[i].freq_wanted);
              vm.hammer[i].freq_hw = vm.hammer[i].freq_wanted;
           }
         }
         printf("PLL change middle 2\n"); 
         usleep(2000);
         enable_good_engines_all_asics_ok();
         resume_all_work();
         end_stopper(&tv,"PLL change");        
         vm.pll_changed=0;
         squid_wait_hammer_reads();
       }
 
 // Finalise last PLL setting
 if (vm.hammer[pll_set_addr].asic_present && 
     vm.hammer[pll_set_addr].pll_waiting_reply) {
   enable_engines_asic(vm.hammer[pll_set_addr].address, vm.hammer[pll_set_addr].working_engines);
   vm.hammer[pll_set_addr].pll_waiting_reply = false;
   printf("Pll done:%x \n",vm.hammer[pll_set_addr].address);
 }

  // find next PLL
  pll_set_addr = (pll_set_addr+1)%HAMMERS_COUNT;
  /*
  for (int i = 0 ; i < 20 ; i++) {
    if (!vm.hammer[pll_set_addr].asic_present ||
     (vm.hammer[pll_set_addr].freq_wanted == vm.hammer[pll_set_addr].freq_hw)) {
      pll_set_addr = (pll_set_addr+1)%HAMMERS_COUNT;
    } else {
      break;
    }
  }
  */

  // prepare next PLL - start pll change
  if (vm.hammer[pll_set_addr].asic_present && 
      (vm.hammer[pll_set_addr].freq_wanted != vm.hammer[pll_set_addr].freq_hw)) {
    disable_engines_asic(pll_set_addr);
    //doing_pll = 1;
    //printf("once_33_msec_tasks_rt set pll %x", pll_set_addr);
    //printf("Set pll %x %d to %d\n",next_pll,vm.hammer[next_pll].freq_hw,vm.hammer[next_pll].freq_wanted);
    set_pll(pll_set_addr, vm.hammer[pll_set_addr].freq_wanted);
    vm.hammer[pll_set_addr].pll_waiting_reply = true;
  }
  // Handle previous READs/WRITEs
  //squid_wait_hammer_reads();
}
#endif  // Change ALL PLLS at once
  
void update_plls_a() {
  int i;
  static int mememe = 0;
  mememe++;
  //printf("Set pll A\n");
  for (i = 0; i < HAMMERS_COUNT; i++) {
    if (vm.hammer[i].asic_present &&
         ((vm.hammer[i].freq_wanted != vm.hammer[i].freq_hw)) /*|| ((mememe%HAMMERS_COUNT) == i)*/) {
      disable_engines_asic(i);
      //printf("Set pll %x %d to %d\n",i,vm.hammer[i].freq_hw,vm.hammer[i].freq_wanted);
      set_pll(i, vm.hammer[i].freq_wanted);
      //vm.hammer[i].freq_hw = vm.hammer[i].freq_wanted;
      vm.hammer[i].pll_waiting_reply = true;
      //write_reg_device(i, ADDR_CONTROL_SET1, BIT_CTRL_DISABLE_TX);
    }
  }
}

void update_plls_b() {
  int i;
  //printf("Set pll B\n");
  for (i = 0; i < HAMMERS_COUNT; i++) {
    if (vm.hammer[i].asic_present && vm.hammer[i].pll_waiting_reply) {
        enable_engines_asic(vm.hammer[i].address, vm.hammer[i].working_engines);
        vm.hammer[i].pll_waiting_reply = false;
        //write_reg_device(i, ADDR_CONTROL_SET0, BIT_CTRL_DISABLE_TX);
    }
  }
}

void once_26_msec_temp_rt() {
  static uint32_t intr_reg=0;
  static int measure_temp_addr = 0;
  uint32_t win_reg =0;

#if 0   
  if (vm.hammer[pll_set_addr].asic_present && 
      vm.hammer[pll_set_addr].pll_waiting_reply) {
    enable_engines_asic(vm.hammer[pll_set_addr].address, vm.hammer[pll_set_addr].working_engines);
    vm.hammer[pll_set_addr].pll_waiting_reply = false;
    write_reg_device(pll_set_addr, ADDR_CONTROL_SET0, BIT_CTRL_DISABLE_TX);
  }

 

   // find next PLL   
   pll_set_addr = (pll_set_addr+1)%HAMMERS_COUNT;
   for (int i = 0 ; i < 20 ; i++) {
     if (!vm.hammer[pll_set_addr].asic_present ||
         (vm.hammer[pll_set_addr].freq_wanted == vm.hammer[pll_set_addr].freq_hw)) {
       pll_set_addr = (pll_set_addr+1)%HAMMERS_COUNT;
     } else {
       break;
     }
   }
   

   // prepare next PLL - start pll change  
   if (vm.hammer[pll_set_addr].asic_present && 
       (vm.hammer[pll_set_addr].freq_wanted != vm.hammer[pll_set_addr].freq_hw)) {
     disable_engines_asic(pll_set_addr);
     //doing_pll = 1;
     //printf("once_33_msec_tasks_rt set pll %x", pll_set_addr);
     printf("Set pll %x %d to %d\n",pll_set_addr,vm.hammer[pll_set_addr].freq_hw,vm.hammer[pll_set_addr].freq_wanted);
     set_pll(pll_set_addr, vm.hammer[pll_set_addr].freq_wanted);
     write_reg_device(pll_set_addr, ADDR_CONTROL_SET1, BIT_CTRL_DISABLE_TX);
     vm.hammer[pll_set_addr].pll_waiting_reply = true;
   }
#endif  
  push_hammer_read(BROADCAST_ADDR, ADDR_BR_WIN, &win_reg);

#if 1
  measure_temp_addr = (measure_temp_addr+1)%HAMMERS_COUNT;
  if(vm.hammer[measure_temp_addr].asic_present) {
    set_temp_reading_rt( measure_temp_addr, &intr_reg); 
  }
#endif  
  
//-------------------------------
 squid_wait_hammer_reads();
//-------------------------------

#if 1
  // Update temperature.
  if (vm.hammer[measure_temp_addr].asic_present) {
      // will be NULL on first run, not a problem
      proccess_temp_reading_rt(&vm.hammer[measure_temp_addr], intr_reg);
  } 
#endif

// read last time...
  while (win_reg) {
         //struct timeval tv;;
       //start_stopper(&tv);
       uint16_t winner_device = BROADCAST_READ_ADDR(win_reg);
       vm.hammer[winner_device].solved_jobs++;
       vm.solved_jobs_total++;
       vm.solved_difficulty_total += 1<<(vm.cur_leading_zeroes-32);
       //printf("WON:%x\n", winner_device);
       win_reg = get_print_win(winner_device);
       //end_stopper(&tv,"WIN STOPPER");
       push_job_to_hw_rt();
  }

}




void push_job_to_hw_rt() {
  RT_JOB work;
  RT_JOB *actual_work = NULL;
  int has_request = pull_work_req(&work);
  if (has_request) {
    // Update leading zeroes?
    vm.not_mining_time = 0;
    if (work.leading_zeroes != vm.cur_leading_zeroes) {
      if (work.leading_zeroes > MAX_LEADING_ZEROES) {
        work.leading_zeroes = MAX_LEADING_ZEROES;
      }
      vm.cur_leading_zeroes = work.leading_zeroes;      
      write_reg_broadcast(ADDR_WIN_LEADING_0, vm.cur_leading_zeroes);
    }
    //flush_spi_write();
#if 1
    if (work.ntime_offset) {
      //work.ntime_offset++;
      work.timestamp = ntohl(ntohl(work.timestamp) + work.ntime_offset);  
      //work.timestamp += work.ntime_offset;
    }
#endif     
    actual_work = add_to_sw_rt_queue(&work);
    // write_reg_device(0, ADDR_CURRENT_NONCE_START, rand() + rand()<<16);
    // write_reg_device(0, ADDR_CURRENT_NONCE_START + 1, rand() + rand()<<16);
    push_to_hw_queue_rt(actual_work);
    vm.last_second_jobs++;
    if (vm.cosecutive_jobs < MAX_CONSECUTIVE_JOBS_TO_COUNT) {    
      vm.cosecutive_jobs++;
    }
  } else {    
    vm.slow_asic_start = 1;
    if (vm.cosecutive_jobs > 0) {
      vm.cosecutive_jobs--;
    }
  }

}



void ping_watchdog() {
    FILE *f = fopen("/var/run/dont_reboot", "w");
    if (!f) {
      psyslog("Failed to create watchdog file\n");
      return;
    }
    fprintf(f, "%d\n", vm.total_mhash);
    fclose(f);
}



void save_rate_temp(int back_tmp, int front_tmp, int total_mhash) {
    FILE *f = fopen("/var/run/mg_rate_temp", "w");
    if (!f) {
      psyslog("Failed to create watchdog file\n");
      return;
    }
    fprintf(f, "%d %d %d\n", total_mhash, back_tmp, front_tmp);
    fclose(f);
}

void wait_dll_ready() {
   int dll;
   int i = 0;
   do {
     i++;
     dll = read_reg_broadcast(ADDR_BR_PLL_NOT_READY);
   } while (dll != 0 && (i < 200));

   //printf("DLL i=%d\n",i);
}


// 666 times a second
void once_2000_usec_tasks_rt() {
  static int counter = 0;
  //start_stopper(&tv);
#if COUNT_IDLES    
  static uint32_t idle=0;
#endif
  counter++;

  //squid_wait_hammer_reads();
  // Enable for debug.
#if COUNT_IDLES  
  push_hammer_read(BROADCAST_ADDR, ADDR_BR_CONDUCTOR_IDLE, &idle);
#endif
  
  //static int fifo_empty; 
  //read_reg_broadcast(ADDR_BR_FIFO_EMPTY);
  //push_hammer_read(BROADCAST_ADDR, ADDR_BR_FIFO_EMPTY, &fifo_empty);
  if (!vm.asics_shut_down_powersave /*&& (fifo_empty)*/) {
    push_job_to_hw_rt();
  }


  if (counter % (500) == 0) {
    //struct timeval tv;     
    //start_stopper(&tv);
    once_second_tasks_rt();
    //end_stopper(&tv,"A");
    push_job_to_hw_rt();

    // Push one more job!
  }

  if (counter % 13 == 10) {
    struct timeval tv;     
    once_26_msec_temp_rt();
  }

  // The PLL like to lock slowly
  if (((counter % 500 == 20) && vm.needs_scaling) || 
     (vm.last_bist_state_machine == BIST_SM_CHANGE_FREQ1)) {
     //struct timeval tv;     
     //start_stopper(&tv);
     update_plls_a();
     if (vm.last_bist_state_machine == BIST_SM_CHANGE_FREQ1) {
       vm.last_bist_state_machine = BIST_SM_DO_BIST_AGAIN;
     }
     push_job_to_hw_rt();     
     wait_dll_ready();
     push_job_to_hw_rt();
     update_plls_b();
     //end_stopper(&tv,"PLL Update B");
     push_job_to_hw_rt();
     vm.needs_scaling = 0;
  }
   // Enable for debug.
   squid_wait_hammer_reads();
#if COUNT_IDLES     
   if (idle) {
      if (BROADCAST_READ_ADDR(idle) != pll_set_addr) {
        vm.idle_probs++;
      } else if (vm.cosecutive_jobs) {
        //printf("FAKE IDLE::::<3:::%x\n", idle);
      }
    } else {
      vm.busy_probs++;
    }
#endif 
  //end_stopper(&tv,"3x3x");
}


void maybe_change_freqs_nrt();

// never returns - thread
void *i2c_state_machine_nrt(void *p) {
  struct timeval tv;
  struct timeval last_job_pushed;
  static int mgmt_tmp;
  static int bottom_tmp;
  static int top_tmp;
  int counter = 0;
  for (;;) {
    counter++;
    if (((counter%4)==0)) {
      update_vm_with_currents_and_temperatures_nrt();
      if (kill_app) {
        printf("NRT out\n");
        return NULL;
      }

      
      if (((counter%12)==0)) {
           leds_periodic_quater_second();
      }

#if 0
      if(vm.last_win.winner_nonce) {
         static unsigned char hash[32];
         compute_hash((const unsigned char*)vm.last_win.midstate,
                       SWAP32(vm.last_win.mrkle_root),
                       SWAP32(vm.last_win.timestamp),
                       SWAP32(vm.last_win.difficulty),
                       SWAP32(vm.last_win.winner_nonce),
                       hash);
        int leading_zeroes = get_leading_zeroes(hash);    
        int winner_device = vm.last_win.winner_device;
        DBG(DBG_WINS,"Win leading Zeroes: %d\n", leading_zeroes);
        if (leading_zeroes < 15) {
           psyslog("!!!!!!!! FP on %d[%d] loop %d (Lz=%x)\n", 
                    winner_device,vm.last_win.winner_engine, winner_device/HAMMERS_PER_LOOP, leading_zeroes);
           vm.false_positives_total++;
           // down ASICTreat it like failed BIST
           vm.hammer[winner_device].passed_last_bist_engines &= 0x7EFE;
        } else {
          psyslog("WIN OK\n");
        }
          
        vm.last_win.winner_nonce  = 0;
        vm.last_win.winner_device = 0;
      }
#endif
    }

  

    // Update AC2DC once every second
    if ((counter % (48)) ==  0)  {
      //printf("BOARD TEMP = %d\n", get_mng_board_temp());
      //printf("BOTTOM TEMP = %d\n", get_bottom_board_temp());
      //printf("TOP TEMP = %d\n", get_top_board_temp());       
      update_ac2dc_power_measurments();
      maybe_change_freqs_nrt();

      if ((counter % (48)*2) ==  0)  {
        print_scaling();
        if (vm.last_second_jobs == 0) {
          vm.not_really_mining_seconds++;
          if (vm.not_really_mining_seconds == 60) {
            exit_nicely(1);
          }
        } else {
          vm.not_really_mining_seconds = 0;
        }
        vm.last_second_jobs = 0;
      }



      if ((counter % (48*12)) ==  0)  {
        int err;
        
        mgmt_tmp = get_mng_board_temp();
        // If temperature dropped by more then 3 degree - 
        // reset mining settings
        if (vm.mgmt_temp_max < mgmt_tmp) {
           vm.mgmt_temp_max = mgmt_tmp;
        }
        
        if ((vm.mgmt_temp_max - mgmt_tmp) >= 3) {
          psyslog("Temperature dropped by more then 3 deg - recalibrate miner!\n");
          for (int addr = 0; addr < HAMMERS_COUNT; addr++) {
            vm.loop[addr/HAMMERS_PER_LOOP].crit_temp_downscale = 0;
            HAMMER *a = &vm.hammer[addr];
            if (a->asic_present) {
              vm.hammer[addr].freq_thermal_limit = (ASIC_FREQ)(vm.hammer[addr].freq_bist_limit);
              vm.mgmt_temp_max = mgmt_tmp;
            }
          }
        }
        
        bottom_tmp = get_bottom_board_temp();
        top_tmp = get_top_board_temp();      
        printf("MGMT TEMP = %d\n",mgmt_tmp);
        printf("BOTTOM TEMP = %d\n",bottom_tmp);
        printf("TOP TEMP = %d\n",top_tmp);


        if ((mgmt_tmp > MAX_MGMT_TEMP_FANS && vm.max_fan_level < 90)) {
          psyslog("Critical temperature - turn fans on!\n");
          vm.max_fan_level = 90;
          set_fan_level(100);
          static char x[200]; 
          sprintf(x, "Thermal FAN UP!");
          mg_event(x);
        }
        

        
        if ((mgmt_tmp > MAX_MGMT_TEMP) || (bottom_tmp > MAX_BOTTOM_TEMP) || (top_tmp > MAX_TOP_TEMP)) {
          psyslog("Critical temperature - exit!\n");
          set_light(LIGHT_GREEN, LIGHT_MODE_OFF);          
          set_fan_level(100);
          static char x[200]; 
          sprintf(x, "Thermal shut down!");
          mg_event(x);
          exit_nicely(120);
          // Never returns...
        }
        
      }
      


      // Every 10 seconds save "mining" status
      if ((counter % (48*20)) ==  0)  {
        if (vm.cosecutive_jobs > 0) {
          ping_watchdog();
        }
      } 
        
      // Every 11 seconds save "mining" status
      if ((counter % (48*11)) ==  0)  {
        save_rate_temp(bottom_tmp, mgmt_tmp,(vm.cosecutive_jobs)?vm.total_mhash:0);
      }




      // Once every minute
      if (counter%(48*60) == 0) {
        static int addr = 0;
        // bring up one asic thermal limit one minute in case winter came.
        addr = (addr+1)%HAMMERS_COUNT;
        if (vm.hammer[addr].asic_present && 
          (vm.hammer[addr].freq_thermal_limit < vm.hammer[addr].freq_bist_limit) &&
           vm.hammer[addr].asic_temp < (vm.max_asic_temp-1)) {
          vm.hammer[addr].freq_thermal_limit = (ASIC_FREQ)(vm.hammer[addr].freq_thermal_limit+1);
        }
        //vm.solved_difficulty_total = 0;    


        if (counter%(48*60*2) == 0) {
          ac2dc_scaling();
        }
        
        // once an hour - increase max vtrim on one loop
        if (counter%(48*60*60) == 0) { 
          static int loop = 0;
          spond_save_nvm();
          loop = (loop+1)%LOOP_COUNT;
          if (vm.loop[loop].dc2dc.max_vtrim_currentwise < vm.vtrim_max) {
            vm.loop[loop].dc2dc.max_vtrim_currentwise = vm.loop[loop].dc2dc.max_vtrim_currentwise+1;
          } 
        }
      }


   
       
    }

  

  
    usleep(1000000/48);
  }
}


// never returns - thread
void *squid_regular_state_machine_rt(void *p) {
  int loop = 0;
  psyslog("Starting squid_regular_state_machine_rt!\n");
  flush_spi_write();
  //return NULL;
  // Do BIST before start
  hammer_iter hi;
  hammer_iter_init(&hi);
  vm.not_mining_time = 0;
  unpause_all_mining_engines();
  pause_all_mining_engines();
  /*
  static struct sched_param param;
  param.sched_priority = sched_get_priority_max(SCHED_RR);
  int r = sched_setscheduler(0, SCHED_RR, &param);
  passert(r==0);
  */  

/*
  while(1) {
    update_ac2dc_power_measurments();
  }
  */
  

  struct timeval tv;
  struct timeval last_job_pushed;
  struct timeval last_force_queue;
  gettimeofday(&last_job_pushed, NULL);
  gettimeofday(&last_force_queue, NULL);
  gettimeofday(&tv, NULL);
  int usec;
  vm.last_bist_state_machine = BIST_SM_DO_BIST_AGAIN;

  for (;;) {
    gettimeofday(&tv, NULL);
    usec = (tv.tv_sec - last_job_pushed.tv_sec) * 1000000;
    usec += (tv.tv_usec - last_job_pushed.tv_usec);
    //struct timeval * tv2;

    if (usec >= JOB_PUSH_PERIOD_US) { 
      // new job every 1.5 msecs = 660 per second
      pthread_mutex_lock(&hammer_mutex);
      //start_stopper(&tv);

      static int fifo_empty = 1; 
      //struct timeval  tv;
      //start_stopper(&tv);
      //fifo_empty = read_reg_broadcast(ADDR_BR_FIFO_EMPTY);
      //end_stopper(&tv,"RRR");
      //push_hammer_read(BROADCAST_ADDR, ADDR_BR_FIFO_EMPTY, &fifo_empty);
      if (fifo_empty) {
        once_2000_usec_tasks_rt();
      }
      //end_stopper(&tv,"WHOOOOOOOOOOOOLE 1500 TIMER");
      pthread_mutex_unlock(&hammer_mutex);

      last_job_pushed = tv;
      int drift = usec - JOB_PUSH_PERIOD_US;
      if (last_job_pushed.tv_usec > drift) {
        last_job_pushed.tv_usec -= drift;
      } 
    } else {
      if (kill_app) {
        printf("RT out\n");
        return NULL;
      }
      usleep(JOB_PUSH_PERIOD_US - usec);
    }
  }
}
