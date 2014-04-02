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

#ifndef _____HAMMERLIBBBB_H____
#define _____HAMMERLIBBBB_H____

#include "nvm.h"
#include "hammer.h"

void pause_all_mining_engines();
void unpause_all_mining_engines();

typedef struct {
  int l;
  int h;
  int addr;
  int done;
  HAMMER *a;
} hammer_iter;
//#define HAMMER_ITER_INITIALIZER  {-1, 0, -1, 0, NULL}

typedef struct {
  int l;
  int done;
} loop_iter;

void hammer_iter_init(hammer_iter *e);
int hammer_iter_next_present(hammer_iter *e);
void loop_iter_init(loop_iter *e);
int loop_iter_next_enabled(loop_iter *e);

void stop_all_work_rt();
void resume_all_work();

uint32_t read_reg_broadcast_test(uint8_t offset);
extern int assert_serial_failures;
int one_done_sw_rt_queue(RT_JOB *work);

void *i2c_state_machine_nrt(void *p);
void *squid_regular_state_machine_rt(void *p);
int init_hammers();
int do_bist_ok_rt(int long_bist, int one_speed_above=0);
uint32_t crc32(uint32_t crc, const void *buf, size_t size);
void set_safe_voltage_and_frequency();
void push_hammer_read(uint32_t addr, uint32_t offset, uint32_t *p_value);
void push_hammer_write(uint32_t addr, uint32_t offset, uint32_t value);

int enable_good_loops_ok();
int allocate_addresses_to_devices();
void set_nonce_range_in_engines(unsigned int max_range);
void enable_all_engines_all_asics();

#endif
