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


#include "ac2dc_const.h"
#include "ac2dc.h"
#include "dc2dc.h"
#include "i2c.h"
#include "hammer.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 

extern MINER_BOX vm;
extern pthread_mutex_t i2c_mutex;

void do_stupid_i2c_workaround() {
   //system("i2cdetect -y -r 0 >> /dev/null");
}
static int ac2dc_type = 0; // 0=murata, 1=em1000, 2=em1200
static int eeprom_addr[3] = {AC2DC_MURATA_I2C_EEPROM_DEVICE, AC2DC_EMERSON_I2C_EEPROM_DEVICE, AC2DC_EMERSON_1200_I2C_EEPROM_DEVICE};
static int mgmt_addr[3] = {AC2DC_MURATA_I2C_MGMT_DEVICE, AC2DC_EMERSON_I2C_MGMT_DEVICE, AC2DC_EMERSON_1200_I2C_MGMT_DEVICE};

int ac2dc_getint(int source) {
  int n = (source & 0xF800) >> 11;
  int negative = false;
 // printf("N=%d\n", n);
  // This is shitty 2s compliment on 5 bits.
  if (n & 0x10) {
    negative = true;
    n = (n ^ 0x1F) + 1;
  }
  int y = (source & 0x07FF);
   // printf("Y=%d\n", y);
   // printf("RES:%d\n", (y * 1000) / (1 << n));
  if (negative)
    return (y * 1000) / (1 << n);
  else
    return (y * 1000) * (1 << n);
}

// Return Watts
static int ac2dc_get_power() {
  int err = 0;
  static int warned = 0;
  int r;

  //do_stupid_i2c_workaround();
  r = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_POUT_WORD, &err);
  r = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_POUT_WORD, &err);
  r = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_POUT_WORD, &err);
  r = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_POUT_WORD, &err);
  r = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_POUT_WORD, &err);
  r = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_POUT_WORD, &err);
  r = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_POUT_WORD, &err);
  r = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_POUT_WORD, &err);
  r = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_POUT_WORD, &err);
  r = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_POUT_WORD, &err);
  r = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_POUT_WORD, &err);
  r = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_POUT_WORD, &err);
  r = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_POUT_WORD, &err);
  r = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_POUT_WORD, &err);
  r = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_POUT_WORD, &err);
  r = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_POUT_WORD, &err);
  r = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_POUT_WORD, &err);

  if (err) {
/*    psyslog("RESET I2C BUS?\n");
    system("echo 111 > /sys/class/gpio/export");
    system("echo out > /sys/class/gpio/gpio111/direction");
    system("echo 0 > /sys/class/gpio/gpio111/value");
    usleep(1000000);
    system("echo 111 > /sys/class/gpio/export"); */
    passert(0);
  }
  int power = ac2dc_getint(r); //TODOZ

  fprintf(stderr,"AC2DC Power %d (i2c val=%d)\n" , power , r);


  if (err) {
    if ((warned++) < 10)
      psyslog( "FAILED TO INIT AC2DC\n" );
    if ((warned) == 9)
      psyslog( "FAILED TO INIT AC2DC, giving up :(\n" );
     //pthread_mutex_unlock(&i2c_mutex);
    return 100;
  }
  //pthread_mutex_unlock(&i2c_mutex);

  return power;
}


void ac2dc_init(int* input_voltage) {
  int err;
  i2c_write(PRIMARY_I2C_SWITCH, PRIMARY_I2C_SWITCH_AC2DC_PIN | PRIMARY_I2C_SWITCH_DEAULT);
  int res = i2c_read_word(AC2DC_EMERSON_I2C_MGMT_DEVICE, AC2DC_I2C_READ_TEMP1_WORD, &err);
  if (!err) {
#ifdef MINERGATE
    psyslog("EMERSON 1000 AC2DC LOCATED\n");
#endif
    ac2dc_type = 1;
  } else {
    // NOT EMERSON 1000
    res = i2c_read_word(AC2DC_EMERSON_1200_I2C_MGMT_DEVICE, AC2DC_I2C_READ_TEMP1_WORD, &err);
    if (!err) {
#ifdef MINERGATE
    	psyslog("EMERSON 1200 AC2DC LOCATED\n");
#endif
      ac2dc_type = 2;
    } else {
      // NOT EMERSON 1200
      res = i2c_read_word(AC2DC_MURATA_I2C_MGMT_DEVICE, AC2DC_I2C_READ_TEMP1_WORD, &err);
      if (!err) {
#ifdef MINERGATE
       psyslog("MURATA AC2DC LOCATED\n");
#endif
       ac2dc_type = 0;
      } else {
        // NOT MURATA 1200
        psyslog("UNKNOWN AC2DC - ASSERT\n");
        assert(0);
      }
    }
  }
  
  *input_voltage = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_VIN_WORD, &err);
  *input_voltage = ac2dc_getint(*input_voltage)/1000;

#ifdef MINERGATE
  psyslog("INPUT VOLTAGE=%d\n", *input_voltage);
#endif

 
  i2c_write(PRIMARY_I2C_SWITCH, PRIMARY_I2C_SWITCH_DEAULT);
}


// Return Watts
static int ac2dc_get_temperature() {
  static int warned = 0;
  //psyslog("%s:%d\n",__FILE__, __LINE__);


  int err = 0;
  //i2c_write(PRIMARY_I2C_SWITCH, PRIMARY_I2C_SWITCH_AC2DC_PIN | PRIMARY_I2C_SWITCH_DEAULT);
  do_stupid_i2c_workaround();
  int res = i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_TEMP1_WORD, &err);
  if (err) {

  }

  int temp1 = ac2dc_getint(res);
  if (err) {
    psyslog(RED "ERR reading AC2DC temp\n" RESET);
    // pthread_mutex_unlock(&i2c_mutex);
    return AC2DC_TEMP_GREEN_LINE - 1;
  }
  //do_stupid_i2c_workaround();
  int temp2 = ac2dc_getint(
      i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_TEMP2_WORD, &err));
  //do_stupid_i2c_workaround(); 
  int temp3 = ac2dc_getint(
      i2c_read_word(mgmt_addr[ac2dc_type], AC2DC_I2C_READ_TEMP3_WORD, &err));
  if (temp2 > temp1)
    temp1 = temp2;
  if (temp3 > temp1)
    temp1 = temp3;

  //i2c_write(PRIMARY_I2C_SWITCH, PRIMARY_I2C_SWITCH_DEFAULT);

  return temp1 / 1000;
}

#ifndef MINERGATE
int ac2dc_get_vpd(ac2dc_vpd_info_t *pVpd) {

	int rc = 0;
	int err = 0;

  int pnr_offset = 0x34;
  int pnr_size = 15;
  int model_offset = 0x57;
  int model_size = 4;
  int serial_offset = 0x5b;
  int serial_size = 10;
  int revision_offset = 0x61;
  int revision_size = 2;


  if (ac2dc_type == 0) // MURATA
  {
	  pnr_offset = 0x1D;
	  pnr_size = 21;
	  model_offset = 0x16;
	  model_size = 6;
	  serial_offset = 0x34;
	  serial_size = 12;
	  revision_offset = 0x3a;
	  revision_size = 2;
  }else if (ac2dc_type == 2) // EMRSN1200
  {
	  /*
	   *
5 (0x2c , 0x30) Vendor ID
12 (0x32, 0x3d) Product Name
12 (0x3F , 0x4A ) Product NR
2  (0x4c ,0x4d) REV
13 (0x4f , 0x5B)SNR
	   *
	   */
	  pnr_offset = 0x3F;
	  pnr_size = 12;
	  model_offset = 0x3F;
	  model_size = 12;
	  serial_offset = 0x4f;
	  serial_size = 13;
	  revision_offset = 0x4c;
	  revision_size = 2;

  }

  if (NULL == pVpd) {
    psyslog("call ac2dc_get_vpd performed without allocating sturcture first\n");
    return 1;
  }
  //psyslog("%s:%d\n",__FILE__, __LINE__);
  

  pthread_mutex_lock(&i2c_mutex);

  i2c_write(PRIMARY_I2C_SWITCH, PRIMARY_I2C_SWITCH_AC2DC_PIN | PRIMARY_I2C_SWITCH_DEAULT , &err);

  if (err) {
    fprintf(stderr, "Failed writing to I2C address 0x%X (err %d)",
            PRIMARY_I2C_SWITCH, err);
     pthread_mutex_unlock(&i2c_mutex);
    return err;
  }

  for (int i = 0; i < pnr_size; i++) {
    pVpd->pnr[i] = ac2dc_get_eeprom_quick(pnr_offset + i, &err);
    if (err)
      goto ac2dc_get_eeprom_quick_err;
  }

  for (int i = 0; i < model_size; i++) {
    pVpd->model[i] = ac2dc_get_eeprom_quick(model_offset + i, &err);
    if (err)
      goto ac2dc_get_eeprom_quick_err;
  }

  for (int i = 0; i < revision_size; i++) {
    pVpd->revision[i] = ac2dc_get_eeprom_quick(revision_offset + i, &err);
    if (err)
      goto ac2dc_get_eeprom_quick_err;
  }


  for (int i = 0; i < serial_size; i++) {
    pVpd->serial[i] = ac2dc_get_eeprom_quick(serial_offset + i, &err);
    if (err)
      goto ac2dc_get_eeprom_quick_err;
  }


ac2dc_get_eeprom_quick_err:

  if (err) {
    fprintf(stderr, RED            "Failed reading AC2DC eeprom (err %d)\n" RESET, err);
    rc =  err;
  }

  i2c_write(PRIMARY_I2C_SWITCH, PRIMARY_I2C_SWITCH_DEAULT);
  pthread_mutex_unlock(&i2c_mutex);

  return rc;
}
#endif

// this function assumes as a precondition
// that the i2c bridge is already pointing to the correct device
// needed to read ac2dc eeprom
// no side effect either
// use this funtion when performing serial multiple reads
#ifndef MINERGATE
unsigned char ac2dc_get_eeprom_quick(int offset, int *pError) {


  //pthread_mutex_lock(&i2c_mutex); no lock here !!

  unsigned char b =
      (unsigned char)i2c_read_byte(eeprom_addr[ac2dc_type], offset, pError);

  return b;
}
#endif

// no precondition as per i2c
// and thus sets switch first,
// and then sets it back
// side effect - it closes the i2c bridge when finishes.
#ifndef MINERGATE
int ac2dc_get_eeprom(int offset, int *pError) {
  // Stub for remo
  
  //printf("%s:%d\n",__FILE__, __LINE__);
   pthread_mutex_lock(&i2c_mutex);
  int b;
  i2c_write(PRIMARY_I2C_SWITCH, PRIMARY_I2C_SWITCH_AC2DC_PIN | PRIMARY_I2C_SWITCH_DEAULT, pError);
  if (pError && *pError) {
     pthread_mutex_unlock(&i2c_mutex);
    return *pError;
  }

  b = i2c_read_byte(eeprom_addr[ac2dc_type], offset, pError);
  i2c_write(PRIMARY_I2C_SWITCH, PRIMARY_I2C_SWITCH_DEAULT);
   pthread_mutex_unlock(&i2c_mutex);
  return b;
}
#endif

void reset_i2c() {
  FILE *f = fopen("/sys/class/gpio/export", "w");
  if (!f)
    return;
  fprintf(f, "111");
  fclose(f);
  f = fopen("/sys/class/gpio/gpio111/direction", "w");
  if (!f)
    return;
  fprintf(f, "out");
  fclose(f);
  f = fopen("/sys/class/gpio/gpio47/value", "w");
  if (!f)
    return;
  fprintf(f, "0");
  usleep(1000000);
  fprintf(f, "1");
  usleep(1000000);
  fclose(f);
}



#ifdef MINERGATE
int update_ac2dc_power_measurments() {
#if AC2DC_BUG == 0
	fprintf(stderr,"REAL update_ac2dc_power_measurments");
  int err;
  static int counter = 0;
  counter++;
  pthread_mutex_lock(&i2c_mutex);
  //reset_i2c();
  i2c_write(PRIMARY_I2C_SWITCH, PRIMARY_I2C_SWITCH_AC2DC_PIN | PRIMARY_I2C_SWITCH_DEAULT);  
  //vm.ac2dc_temp = ac2dc_get_temperature();
  //int power_guessed = (vm.dc2dc_total_power*1000/790)+60;// ac2dc_get_power()/1000; //TODOZ
  //int power = power_guessed;
  if (vm.cosecutive_jobs >= MIN_COSECUTIVE_JOBS_FOR_AC2DC_MEASUREMENT) {
      vm.ac2dc_power = ac2dc_get_power()/1000;
    } else {
      vm.ac2dc_power = 0;
    }
  i2c_write(PRIMARY_I2C_SWITCH, PRIMARY_I2C_SWITCH_AC2DC_PIN | PRIMARY_I2C_SWITCH_DEAULT);  
  pthread_mutex_unlock(&i2c_mutex);  
#else 
  printf(stderr,"FAKGE update_ac2dc_power_measurments");
  if (vm.cosecutive_jobs >= MIN_COSECUTIVE_JOBS_FOR_AC2DC_MEASUREMENT) {
     vm.ac2dc_power = (vm.dc2dc_total_power*1000/790)+60;;
  } else {
     vm.ac2dc_power = 0;
  }
#endif
  return 0;
}
#endif

