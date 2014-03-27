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
    printf("Usage: %s <outfile>\n" , argv[0]);
    return 0;
    }
  srand (time(NULL));


  zabbix_log.magic = MG_ZABBIX_LOG_MAGIC;
  zabbix_log.version = MG_ZABBIX_LOG_VERSION;
  zabbix_log.zabbix_logsize = sizeof(zabbix_log);
  zabbix_log.ac2dc_power = 0xbc;
  zabbix_log.ac2dc_temp = 0xbd;
  zabbix_log.time = time(NULL);
  zabbix_log.dump_id = 1;

  for(int l = 0; l < LOOP_COUNT ; l++) {
    zabbix_log.loops[l].voltage = (rand() % 5 + 10); // bet 8 and 12, should be read as between .8 and 1.2.
    zabbix_log.loops[l].current = rand()%8 +16;
    zabbix_log.loops[l].temp = rand() % 70 + 50;
    zabbix_log.loops[l].enabled = rand() % 100 > 95 ? 0 : 1; // 5% disabled
  }

  for(int a = 0; a < HAMMERS_COUNT ; a++) {
    zabbix_log.asics[a].freq = (rand()%400+100);  // 5 - 25, arbitrary range
    zabbix_log.asics[a].temp = rand() % 70 + 50;
    zabbix_log.asics[a].wins = rand() % 16 > 14 ? 1 : 0; // difficulty
    zabbix_log.asics[a].working_engines = 92; //  out of 92? engines
    zabbix_log.asics[a].failed_bists = rand() % 2; //
    zabbix_log.asics[a].freq_time = rand() % 100; // ?? 
  }




  char *file_name = argv[1];
  FILE *fp = fopen(file_name, "w");

  if (fp != NULL)
    {
      fwrite(&zabbix_log,1,sizeof(zabbix_log),fp); 
        fclose(fp);
    } else {
    printf("Failed to load zabbix log\n");
    return -1;    
    }
  return 0;
}


