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
#include <fcntl.h>
#include <execinfo.h>

#include <linux/types.h>
#include <time.h>
#include <syslog.h>
#include <spond_debug.h>




#define SIZE 100
#ifdef MINERGATE
void exit_nicely(int seconds_sleep_before_exit, const char* why);
#endif

void print_stack() {
	int j, nptrs;
	 void *buffer[SIZE];
	 char **strings;
	 psyslog("ERROR, ABORT, DYE!\n");
	 psyslog("MINERGATE ERROR: ");
	
	 nptrs = backtrace(buffer, SIZE);
	 strings = backtrace_symbols(buffer, nptrs);
	 for (j = 0; j < nptrs; j++) {
	   psyslog(" %s\n", strings[j]);
	 }

}


void _pabort(const char *s) {
	 if (s) {
	   //perror(orig_buf);
	   psyslog("%s :STACK:", s);
	 }
  print_stack();
#ifdef MINERGATE  
  exit_nicely(0, "apAbort");
#else
  exit(0);
#endif
}

void _passert(int cond, const char *s) {
  if (!cond) {
    pabort(s);
  }
}

void start_stopper(struct timeval *tv) {gettimeofday(tv, NULL);}

void end_stopper(struct timeval *tv, const char *name) {
  int usec;
  struct timeval end;
  gettimeofday(&end, NULL);
  usec = (end.tv_sec - tv->tv_sec) * 1000000;
  usec += (end.tv_usec - tv->tv_usec);
  psyslog("%s took %d\n", name, usec);
}


void mg_event(const char *s) {
	FILE *f = fopen("/tmp/mg_event", "w");
    if (!f) {
      psyslog("Failed to create watchdog file\n");
      return;
    }
    fprintf(f, "%s\n", s);
    fclose(f);
}

void mg_status(const char *s) {
	FILE *f = fopen("/tmp/mg_status", "w");
    if (!f) {
      psyslog("Failed to create watchdog file\n");
      return;
    }
    fprintf(f, "%s\n", s);
    fclose(f);

}

