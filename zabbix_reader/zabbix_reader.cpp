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

uint32_t total_temp = 0;
uint32_t total_temp_last = 0;
uint32_t total_freq = 0;
uint32_t total_freq_last = 0;
uint64_t total_cycles = 0;
uint64_t total_cycles_last = 0;
uint32_t total_failed_bists = 0;
uint32_t total_failed_bists_last = 0;

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

  if (zabbix_log.magic != MG_ZABBIX_LOG_MAGIC) {
    printf("zabbix log MAGIC wrong, %x instead of %x\n", zabbix_log.magic , MG_ZABBIX_LOG_MAGIC);
    return -1;
  }

  if (zabbix_log.version != MG_ZABBIX_LOG_VERSION) {
    printf("zabbix log VERSION wrong, %x instead of %x\n", zabbix_log.version , MG_ZABBIX_LOG_VERSION);
    return -1;
  }


  printf("miner.ac2dc.current %d\n", zabbix_log.ac2dc_power);
  printf("miner.ac2dc.temp %d\n", zabbix_log.ac2dc_temp);

  for(int l = 0; l < LOOP_COUNT ; l++) {
    printf("loop%d.voltage %d\n", l, zabbix_log.loops[l].voltage);
    printf("loop%d.current %d\n", l, zabbix_log.loops[l].current);
    printf("loop%d.temp %d\n", l, zabbix_log.loops[l].temp);
    printf("loop%d.enabled %x\n", l, zabbix_log.loops[l].enabled  );
    for(int a = 0; a < (HAMMERS_PER_LOOP); a++){
      printf("loop%d.asic%d.freq %d\n", l, a, zabbix_log.asics[a*l].freq);
      printf("loop%d.asic%d.working_engines %d\n", l, a, zabbix_log.asics[a*l].working_engines);
      printf("loop%d.asic%d.failed_bists %x\n", l, a, zabbix_log.asics[a*l].failed_bists);
      printf("loop%d.asic%d.freq_time %d\n", l, a, zabbix_log.asics[a*l].freq_time);
      printf("loop%d.asic%d.temp %d\n", l, a, zabbix_log.asics[a*l].temp);
                        printf("loop%d.asic%d.wins %x\n", l, a, zabbix_log.asics[a*l].wins);
      printf("loop%d.asic%d.cycles %d\n", l, a, zabbix_log.asics[a*l].working_engines*zabbix_log.asics[l*a].freq);
      total_temp += zabbix_log.asics[a*l].temp;
      total_freq += zabbix_log.asics[a*l].freq;
      total_failed_bists += zabbix_log.asics[a*l].failed_bists;
      total_cycles += (zabbix_log.asics[a*l].working_engines*zabbix_log.asics[l*a].freq);
    }
    printf("loop%d.temp[avg] %f\n", l, float((total_temp-total_temp_last)/HAMMERS_PER_LOOP));
    printf("loop%d.freq[avg] %f\n", l, float((total_freq-total_freq_last)/HAMMERS_PER_LOOP));
    printf("loop%d.cycles[total] %ld\n", l, (total_cycles-total_cycles_last));
    printf("loop%d.failed_bists[total] %d\n", l, (total_failed_bists-total_failed_bists_last));
    total_freq_last=total_freq;
    total_failed_bists_last=total_failed_bists;
    total_temp_last=total_temp;
    total_cycles_last=total_cycles;
  }
  printf("miner.cycles[total] %ld\n", total_cycles);
}


