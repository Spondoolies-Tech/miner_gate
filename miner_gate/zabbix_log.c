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
#include <string.h>
#include <execinfo.h>
#include "hammer.h"
#include <time.h>
#include <mg_proto_parser.h>
#include <spond_debug.h>
#include <json.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "zabbix_log.h"

MG_ZABBIX_LOG zabbix_log;
void update_zabbix_stats() {
  zabbix_log.magic = MG_ZABBIX_LOG_MAGIC;
  zabbix_log.version = MG_ZABBIX_LOG_VERSION;
  zabbix_log.zabbix_logsize = sizeof(zabbix_log);
  zabbix_log.ac2dc_power = vm.ac2dc_power;
  zabbix_log.ac2dc_temp = vm.ac2dc_temp;

  for (int l = 0; l < LOOP_COUNT; l++) {
    zabbix_log.loops[l].current = vm.loop[l].dc2dc.dc_current_16s;
    zabbix_log.loops[l].temp = vm.loop[l].dc2dc.dc_temp;
    zabbix_log.loops[l].enabled = vm.loop[l].enabled_loop;
  }

  int t = time(NULL);
  for (int l = 0; l < HAMMERS_COUNT; l++) {
    zabbix_log.asics[l].freq = vm.hammer[l].freq_wanted;
    zabbix_log.asics[l].temp = vm.hammer[l].asic_temp;
    zabbix_log.asics[l].working_engines = vm.hammer[l].working_engines;
    zabbix_log.asics[l].failed_bists = 0;
    zabbix_log.asics[l].freq_time = t - vm.hammer[l].last_freq_change_time;
  }
}

void dump_zabbix_stats() {
  FILE *fp = fopen("/my_nfs/zabbix_log.bin", "w");
  if (fp != NULL) {
    update_zabbix_stats();
    fwrite(&zabbix_log, 1, sizeof(zabbix_log), fp);
    fclose(fp);
  } else {
    printf("Failed to save zabbix log\n");
  }
}
