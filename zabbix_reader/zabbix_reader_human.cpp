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
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <netinet/in.h>
#include <string.h>
#include <execinfo.h>
#include <time.h>
//#include <json.h>
#include "zabbix_log.h"


MG_ZABBIX_LOG zabbix_log;

int main(int argc, char *argv[]) {
  if (argc < 2 ) {
    printf("Usage: %s <infile>\n" , argv[0]);
    return 0;
    }

  char *file_name = argv[1];
  FILE *fp = fopen(file_name, "r");

  if (fp != NULL)
    {
      fread(&zabbix_log,1,sizeof(zabbix_log),fp); 
        fclose(fp);
    } else {
    printf("Failed to load zabbix log\n");
    return -1;    
    }

#if 0
    zabbix_log.magic = MG_ZABBIX_LOG_MAGIC;
    zabbix_log.version = MG_ZABBIX_LOG_VERSION;
    zabbix_log.ac2dc_power = 0xbc;
    zabbix_log.ac2dc_temp = 0xbd;

    for(int l = 0; l < LOOP_COUNT ; l++) {
      zabbix_log.loops[l].voltage = 1;
      zabbix_log.loops[l].current = 1;
      zabbix_log.loops[l].temp = 1;
    }

    for(int l = 0; l < HAMMERS_COUNT ; l++) {
      zabbix_log.asics[l].freq = 1;
      zabbix_log.asics[l].temp = 1;
    }
#endif
  if (zabbix_log.zabbix_logsize != sizeof(zabbix_log)) {
      printf("zabbix log SIZE is wrong, %x instead of %x\n", 
        (int)zabbix_log.zabbix_logsize , (int)sizeof(zabbix_log));
      return -1;
  }


  if (zabbix_log.magic != MG_ZABBIX_LOG_MAGIC) {
    printf("zabbix log MAGIC is wrong, %x instead of %x\n", zabbix_log.magic , MG_ZABBIX_LOG_MAGIC);
    return -1;
  }

  if (zabbix_log.version != MG_ZABBIX_LOG_VERSION) {
    printf("zabbix log VERSION is wrong, %x instead of %x\n", zabbix_log.version , MG_ZABBIX_LOG_VERSION);
    return -1;
  }





  printf("AC2DC.A %2x\n", zabbix_log.ac2dc_power);
  printf("AC2DC.C %2x\n", zabbix_log.ac2dc_temp);
  printf("DC2DC\n     |");
  for(int l = 0; l < LOOP_COUNT ; l++) {
    printf("%3x ", l);
  }
  printf("\n------");
  for(int l = 0; l < LOOP_COUNT ; l++) {
    printf("---");
  }
  printf("\nVOLT |");
  for(int l = 0; l < LOOP_COUNT ; l++) {
    int j;    
    printf("%3d ", zabbix_log.loops[l].voltage);
  }
  printf("\nCRNT |");
  for(int l = 0; l < LOOP_COUNT ; l++) {
      printf("%3x ", zabbix_log.loops[l].current);
  }
  printf("\nTEMP |");
  for(int l = 0; l < LOOP_COUNT ; l++) {
      printf("%3x ", zabbix_log.loops[l].temp);
  }
  printf("\n------");
  for(int l = 0; l < LOOP_COUNT ; l++) {
    printf("---");
  }
  printf("\n");

  printf("ASIC Freq/Temp/Uptime/Engines:\n");
  printf("   |");
  for(int l = 0; l < HAMMERS_PER_LOOP ; l++) {
    printf(" Hz  C'   Up  E |");
  }
  for(int l = 0; l < HAMMERS_COUNT ; l++) {
    if (l%HAMMERS_PER_LOOP == 0) {
      printf("\n%2d |", l/HAMMERS_PER_LOOP);
    }
    printf("%3d %3d %4d %2x|", 
      zabbix_log.asics[l].freq*15, 
      zabbix_log.asics[l].temp*ASIC_TEMP_DELTA+ASIC_TEMP_MIN,
      zabbix_log.asics[l].freq_time%999,
      zabbix_log.asics[l].working_engines);
  }
  
  printf("\n");
}


