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

#ifndef _____HAMMER_H____
#define _____HAMMER_H____

#include "nvm.h"

#include "pwm_manager.h"

#define dprintf printf

#define SHA256_HASH_BITS 256
#define SHA256_HASH_BYTES (SHA256_HASH_BITS / 8)
#define SHA256_BLOCK_BITS 512
#define SHA256_BLOCK_BYTES (SHA256_BLOCK_BITS / 8)

typedef struct {
  uint32_t h[8];
  uint32_t length;
} sha2_small_common_ctx_t;

/** \typedef sha256_ctx_t
 * \brief SHA-256 context type
* 
 * A variable of this type may hold the state of a SHA-256 hashing process
 */
typedef sha2_small_common_ctx_t sha256_ctx_t;

#define BROADCAST_ADDR 0xffff

//#define FIRST_CHIP_ADDR                 0xAAA
#define ENGINES_PER_ASIC 15

#define ADDR_CHIP_ADDR 0x0
#define ADDR_GOT_ADDR 0x1
////// INTR ADDR_CONTROL BITMASK - use ADDR_CONTROL_SET0 and ADDR_CONTROL_SET1
//////////////////////
#define BIT_CTRL_DISABLE_TX 0x2
#define BIT_CTRL_BIST_MODE 0x4
#define BIT_CTRL_STOP_SERIAL_CHAIN 0x20
////// INTR BITMASK (for 30,31,32,33) ///////////////////
#define ADDR_CONTROL 0x2

////// ADDR_COMMAND BITMASK ///////////////////
#define BIT_CMD_LOAD_JOB 0x01
#define BIT_CMD_END_JOB 0x02
#define BIT_CMD_END_JOB_IF_Q_FULL 0x04
#define BIT_CMD_TS_RESET_0 0x08
#define BIT_CMD_TS_RESET_1 0x10
/////////////////////////////////////////////////////////
#define ADDR_COMMAND 0x3
#define ADDR_RESETING0 0x4
#define ADDR_RESETING1 0x5
#define ADDR_CONTROL_SET0 0x6
#define ADDR_CONTROL_SET1 0x7


#define ADDR_TS_RSTN_0 0xe
#define ADDR_TS_RSTN_1 0xf


#define ADDR_PLL_FIX 0x10
#define ADDR_PLL_CONFIG 0x11
#define ADDR_PLL_ENABLE 0x12
#define ADDR_CLK_ENABLE 0x13
#define ADDR_PLL_BYPASS 0x14
#define ADDR_PLL_TEST_MODE 0x15
#define ADDR_PLL_TEST_CONFIG 0x16
#define ADDR_PLL_DLY_INC 0x17
#define ADDR_SW_OVERRIDE_PLL 0x18

#define ADDR_PLL_POWER_DOWN 0x19
#define ADDR_PLL_RESET 0x1a
#define ADDR_PLL_STATUS 0x1b
#define ADDR_TS_SET_0 0x1c // 0:2 setting, 3: thermal shutdown
#define ADDR_TS_SET_THERMAL_SHUTDOWN_ENABLE 0x8
#define ADDR_TS_SET_1 0x1d
#define ADDR_TS_DATA_0 0x1e
#define ADDR_TS_DATA_1 0x1f

#define ADDR_TS_EN_0 0x20
#define ADDR_TS_EN_1 0x21
#define ADDR_SHUTDOWN_ACTION 0x22 
#define ADDR_PLL_DIV_DISABLE 0x23
#define ADDR_DLL_OVERRIDE_CFG_LOW 0x24
#define ADDR_DLL_OVERRIDE_CFG_HIGH 0x25
#define ADDR_DLL_OFFSET_CFG_LOW 0x26
#define ADDR_DLL_OFFSET_CFG_HIGH 0x27
#define ADDR_DLL_SOFT_RESET 0x28
#define ADDR_DLL_SOFT_ENABLE 0x29
#define ADDR_DLL_UPDATE_ENABLE 0x2A
#define ADDR_DLL_BYP 0x2B
#define ADDR_DLL_MASTER_MSRMNT 0x2C
#define ADDR_DLL_MASTER_LOCKED 0x2D
#define ADDR_TS_TS_0 0x2E
#define ADDR_TS_TS_1 0x2F

////// INTR BITMASK (for 30,31,32,33) ///////////////////
#define BIT_INTR_NO_ADDR 0x0001
#define BIT_INTR_WIN 0x0002
#define BIT_INTR_FIFO_FULL 0x0004
#define BIT_INTR_FIFO_ERR 0x0008
#define BIT_INTR_CONDUCTOR_IDLE 0x0010
#define BIT_INTR_CONDUCTOR_BUSY 0x0020
#define BIT_INTR_RESERVED1 0x0040
#define BIT_INTR_FIFO_EMPTY 0x0080
#define BIT_INTR_BIST_FAIL 0x0100
#define BIT_INTR_THERMAL_SHUTDOWN 0x0200
#define BIT_INTR_NOT_WIN 0x0400 // NOT USED
#define BIT_INTR_0_OVER 0x0800
#define BIT_INTR_0_UNDER 0x1000
#define BIT_INTR_1_OVER 0x2000
#define BIT_INTR_1_UNDER 0x4000
#define BIT_INTR_PLL_NOT_READY 0x8000
//////////////////////

#define ADDR_PLL_RSTN 0x1A

#define ADDR_INTR_MASK 0x30
#define ADDR_INTR_CLEAR 0x31
#define ADDR_INTR_RAW 0x32
#define ADDR_INTR_SOURCE 0x33
#define ADDR_BR_NO_ADDR 0x40
#define ADDR_BR_WIN 0x41
#define ADDR_BR_FIFO_FULL 0x42
#define ADDR_BR_FIFO_ERROR 0x43
#define ADDR_BR_CONDUCTOR_IDLE 0x44
#define ADDR_BR_CONDUCTOR_BUSY 0x45
#define ADDR_BR_CRC_ERROR 0x46
#define ADDR_BR_FIFO_EMPTY 0x47
#define ADDR_BR_BIST_FAIL 0x48
#define ADDR_BR_THERMAL_VIOLTION 0x49
#define ADDR_BR_NOT_WIN 0x4A
#define ADDR_BR_0_OVER 0x4B
#define ADDR_BR_0_UNDER 0x4C
#define ADDR_BR_1_OVER 0x4D
#define ADDR_BR_1_UNDER 0x4E
#define ADDR_BR_PLL_NOT_READY 0x4F

//#define ADDR_BR_BIST_ERROR        0x47

#define ADDR_MID_STATE 0x50
#define ADDR_MERKLE_ROOT  0x58 // 50 - 57, 8'b0100.0XXX, 8  words of 32-bits = 256
#define ADDR_TIMESTEMP 0x59
#define ADDR_DIFFICULTY 0x5A
#define ADDR_WIN_LEADING_0 0x5B
#define ADDR_JOB_ID 0x5C

#define ADDR_WINNER_JOBID_WINNER_ENGINE 0x60
#define ADDR_WINNER_NONCE 0x61 // Sticky forever.
                               //#define ADDR_WINNER_MRKL         0x62
#define ADDR_CURRENT_NONCE 0x63
#define ADDR_LEADER_ENGINE 0x64

#define ADDR_VERSION 0x70
#define ADDR_ENGINES_PER_CHIP_BITS 0x71 // NOT LOG
//#define ADDR_ENGINES_PER_CLUSTER_BITS   0x72
#define ADDR_LOOP_LOG2 0x73

#define ADDR_ENABLE_ENGINE 0x80 // 80 - 9F, 8'b100X.XXXX, 32 words of 32-bits = 1024
#define ADDR_FIFO_OUT 0xA0

/* A0 - BF, 8'b101X.XXXX, 32 words of 32-bits = 1024 */
/*
#define ADDR_FIFO_OUT_MID_STATE_LSB        0xA0
#define ADDR_FIFO_OUT_MID_STATE_MSB        0xA7
#define ADDR_FIFO_OUT_MARKEL            0xA8
#define ADDR_FIFO_OUT_TIMESTEMP            0xA9
#define ADDR_FIFO_OUT_DIFFICULTY        0xAA
 */

#define ADDR_CURRENT_NONCE_START    0xB0 /* B0 - BF, 8'b1011.XXXX, up to 16 words of 32-bits = 512  */
#define ADDR_CURRENT_NONCE_RANGE    0xC3 /* C0 - CF, 8'b1100.XXXX, up to 16 words of 32-bits = 512  */

// READ ONLY
#define ADDR_FIFO_OUT_MID_STATE_LSB 0xA0
#define ADDR_FIFO_OUT_MID_STATE_MSB 0xA7
#define ADDR_FIFO_OUT_MARKEL 0xA8
#define ADDR_FIFO_OUT_TIMESTEMP 0xA9
#define ADDR_FIFO_OUT_DIFFICULTY 0xAA
#define ADDR_FIFO_OUT_WIN_LEADING_0 0xAB
#define ADDR_FIFO_OUT_WIN_JOB_ID 0xAC

#define ADDR_BIST_NONCE_START 0xD0
#define ADDR_BIST_NONCE_RANGE 0xD1
#define ADDR_BIST_NONCE_EXPECTED 0xD2
#define ADDR_EMULATE_BIST_ERROR 0xD3
// BITMASK same as Engine-enable.
#define ADDR_BIST_PASS 0xD4
// Uses regular leading zeroes.

#define TX_FIFO_NOT_FULL 0x01
#define TX_FIFO_NOT_EMPTY 0x02
#define TX_FIFO_OVERFLOW 0x04
#define TX_FIFO_UNDERRUN 0x08
#define RX_FIFO_NOT_FULL 0x10
#define RX_FIFO_NOT_EMPTY 0x20
#define RX_FIFO_OVERFLOW 0x40
#define RX_FIFO_UNDERRUN 0x80

#define WORK_STATE_FREE 0
#define WORK_STATE_HAS_JOB 1


typedef struct {
  uint8_t work_state; //
  uint8_t work_id_in_hw;
  uint8_t adapter_id;
  uint8_t leading_zeroes;
  uint32_t work_id_in_sw;

  uint32_t difficulty;
  uint32_t timestamp; // BIG ENDIAN !!
  uint32_t mrkle_root;
  uint32_t midstate[8];
  uint32_t winner_nonce[2]; // 0 means no nonce. This means we loose 0.00000000001%
                            // results.
  // uint32_t work_state;
  // uint32_t nonce;
  uint8_t ntime_max;
  uint8_t ntime_offset;
} RT_JOB;

#define WAIT_BETWEEN_FREQ_CHANGE 30 // seconds
// 192 ASICs
#define HAMMER_TO_LOOP(H) ((H->address >> 16) & 0xFF)

typedef struct {
  // asic present and used
  // If loop not enabled set to false
  uint8_t asic_present;
  uint8_t initial_bist_done;
  // address - loop*8 + offset
  uint8_t address;
  uint8_t loop_address;
  // Passed engines (set by "do_bist_ok_rt()")
  uint16_t passed_last_bist_engines;

  // Asic temperature/frequency (polled periodicaly)
  ASIC_TEMP asic_temp; // periodic measurments - ASIC_TEMP_
  ASIC_FREQ freq_hw;        // *15 Hz  
  ASIC_FREQ freq_wanted;        // *15 Hz
  ASIC_FREQ freq_thermal_limit; 
  ASIC_FREQ freq_bist_limit;   
  int pll_waiting_reply;
    
  int agressivly_scale_up;
  int last_freq_change_time; // time() when we increased ASIC frequency
  int last_down_freq_change_time;
  // int dc2dc_ac2dc_limit;
  // Set durring initialisation currently enabled engines
  uint32_t working_engines;

  // Jobs solved by ASIC
  uint32_t solved_jobs;
  uint32_t too_hot_temp_counter;

  // Did we already scaled up this ASIC frequency
  uint8_t can_scale_up;
} HAMMER;

#define INDUCTOR_TYPE_WURTH_REGULAR    0
#define INDUCTOR_TYPE_WURTH_DEV        1
#define INDUCTOR_TYPE_VISHAY           2
#define INDUCTOR_TYPE_WURTH_DEV_2      3


// 24 dc2dc
typedef struct {
  uint8_t dc_temp;
  //int dc_current_16s_arr_ptr;
  //int dc_current_16s_arr[4]; // do median - its noisy?
  int dc_current_16s; // in 1/16 of amper. 0 = bad reading
  int dc_current_limit_16s;   
  int dc_power_watts_16s;  
  int last_voltage_change_time;
  int max_vtrim_currentwise;
  unsigned int last_downscale_time;
  //uint32_t loop_voltage;
  uint32_t loop_vtrim;
  bool loop_margin_low;  
  int inductor_type;

  
  // Guessing added current
} DC2DC;


typedef struct {
  uint8_t id;
  // Last time ac2dc scaling changed limit.
  int last_ac2dc_scaling_on_loop;
  uint8_t enabled_loop;
  int asic_temp_sum; // if asics disabled or missing give them fake temp
  int asic_hz_sum; // if asics disabled or missing give them fake temp
  int overheating_asics;
  int down_scale_type;
  
  int asics_failing_bist;
  int asic_count;
  int crit_temp_downscale;
  DC2DC dc2dc;
  int power_throttled;
} LOOP;


typedef struct {
  uint32_t winner_device;    
  uint32_t winner_nonce;
  uint32_t winner_engine;  
  uint32_t winner_id;
  uint32_t mrkle_root;
  uint32_t timestamp;
  uint32_t difficulty;
  uint32_t midstate[8];
} WIN;  


// Global data
#define BIST_SM_NADA          0
#define BIST_SM_DO_SCALING    1
#define BIST_SM_CHANGE_FREQ1  2
#define BIST_SM_CHANGE_FREQ2  3
#define BIST_SM_DO_BIST_AGAIN 4



typedef struct {
  // Fans set to high
  int fan_level;
  uint32_t good_loops;

  int slow_corner;
  int max_asic_temp;

  int mgmt_temp_max;
  int start_mine_time;
  // pll can be changed
  uint8_t engines_disabled;
  uint8_t last_bist_state_machine;
  

  // How many WINs we got
  uint32_t solved_jobs_total;
  uint32_t false_positives_total;  
  uint32_t solved_difficulty_total;
  
  // random checking of ASICs to see utilisation.
  uint32_t idle_probs;
  uint32_t busy_probs;

  // Pause all miner work to save electricity
  uint8_t asics_shut_down_powersave;
  uint32_t not_mining_time; // in seconds how long we are not mining
  uint32_t mining_time; 


  // Stoped all work
  uint8_t stopped_all_work;

  // bollean flag to change PLLS
  int pll_changed;
  int slow_asic_start;
  // jobs right one after another
  int cosecutive_jobs;

  int bist_fatal_err;  
  int bist_current;  
  int bist_voltage;  
  int serial_errors;
 
  // ac2dc current and temperature
  int ac2dc_power;  // in ampers. 0 = bad reading.
  int dc2dc_total_power; 
  int total_mhash; 
  int concecutive_bad_wins;
  uint32_t ac2dc_temp;
  //int work_mode; // 0 = slow, 1 = normal, 2 = turbo
  int max_fan_level;
  int force_freq;
  int voltage_start_top;
  int voltage_start_bot;  
  int vtrim_start_top;
  int vtrim_start_bot;  
  bool vmargin_start;
  int voltage_max;
  int vtrim_max;
  int max_ac2dc_power; 
  int max_dc2dc_current_16s;   
  int last_second_jobs;
  int not_really_mining_seconds;  
  int cur_leading_zeroes;
  // We give less LZ then needed to do faster scaling.

  // When system just started, search optimal speed agressively

  int needs_scaling;
  
  // our ASIC data
  HAMMER hammer[HAMMERS_COUNT];
  //uint32_t working_engines[HAMMERS_COUNT];

  
  // our loop and dc2dc data
  LOOP loop[LOOP_COUNT];
  int overcurrent_loops;

  WIN last_win;  
} MINER_BOX;

extern MINER_BOX vm;


#define JOB_PUSH_PERIOD_US (2000)

#endif
