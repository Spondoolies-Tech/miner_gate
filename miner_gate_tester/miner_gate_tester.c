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

#include "mg_proto_parser.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <time.h> 
#include "squid.h"
#include "spond_debug.h"


int leading_zeroes  = 0x20;
int jobs_per_time   = 50;
int jobs_period   = 50;



void send_minergate_pkt(const minergate_req_packet* mp_req, 
            minergate_rsp_packet* mp_rsp,
            int  socket_fd) {
  int   nbytes;    
  write(socket_fd, (const void*)mp_req, sizeof(minergate_req_packet));
  nbytes = read(socket_fd, (void*)mp_rsp, sizeof(minergate_rsp_packet));  
  passert(nbytes > 0);
 // printf("got %d(%d) bytes\n",mp_rsp->data_length, nbytes);
  passert(mp_rsp->magic == 0xcaf4);
}


int init_socket() {
  struct sockaddr_un address;
  int  socket_fd ;
  socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
   if(socket_fd < 0)
   {
    psyslog("socket() failed\n");
    perror("Err:");
    return 0;
   }
   
   /* start with a clean address structure */
   memset(&address, 0, sizeof(struct sockaddr_un));
   
   address.sun_family = AF_UNIX;
   sprintf(address.sun_path, MINERGATE_SOCKET_FILE);
   
   if(connect(socket_fd, 
        (struct sockaddr *) &address, 
        sizeof(struct sockaddr_un)) != 0)
   {
    psyslog("connect() failed\n");
    perror("Err:");
    return 0;
   }

  return socket_fd;
}





void fill_random_work2(minergate_do_job_req* work) {
  static int id = 1;
  id++;
  memset(work, 0, sizeof(minergate_do_job_req));
  //work->
  work->midstate[0] = (rand()<<16) + rand()+rand();
  work->midstate[1] = (rand()<<16) + rand();
  work->midstate[2] = (rand()<<16) + rand();
  work->midstate[3] = (rand()<<16) + rand();
  work->midstate[4] = (rand()<<16) + rand();
  work->midstate[5] = (rand()<<16) + rand();
  work->midstate[6] = (rand()<<16) + rand();
  work->midstate[7] = (rand()<<16) + rand();
  work->work_id_in_sw = id%0xFFFF;
  work->mrkle_root = (rand()<<16) + rand();
    work->leading_zeroes = leading_zeroes;
  work->timestamp  = (rand()<<16) + rand();
  work->difficulty = (rand()<<16) + rand();
  //work_in_queue->winner_nonce
}


int main(int argc, char* argv[])
{
 if (argc == 4) {
    leading_zeroes = atoi(argv[1]);
    jobs_per_time= atoi(argv[2]);    
    jobs_period= atoi(argv[3]);        
 } else {
    printf("Usage: %s <leading zeroes> <jobs per send> <send period milli>\n", argv[0]);
    return 0;
 }
 

    
 int  socket_fd = init_socket();
 passert(socket_fd != 0);

 
 
 minergate_req_packet* mp_req = allocate_minergate_packet_req(0xca, 0xfe);
 minergate_rsp_packet* mp_rsp = allocate_minergate_packet_rsp(0xca, 0xfe);
 int i;
 int requests = 0;
 int responces = 0;
 int rate = 0;
 int global_rate = 0; 
 int global_rate_cnt = 0; 
 
 //nbytes = snprintf(buffer, 256, "hello from a client");
 srand (time(NULL));

 while (1) {
  passert(jobs_per_time <= MAX_REQUESTS);
     for (i = 0; i < (jobs_per_time); i++) {
        minergate_do_job_req* p = mp_req->req+i;
        fill_random_work2(p);
    requests++;
    //printf("DIFFFFFFF %d\n", p->leading_zeroes);
     }
     mp_req->req_count = jobs_per_time;
     //printf("MESSAGE TO SERVER: %x\n", mp_req->request_id);
     send_minergate_pkt(mp_req,  mp_rsp, socket_fd);
     //printf("MESSAGE FROM SERVER: %x\n", mp_rsp->request_id);

   
    //DBG(DBG_NET, "GOT minergate_do_job_req: %x/%x\n", sizeof(minergate_do_job_req), md->data_length);
    int array_size = mp_rsp->rsp_count;
    int i;
    for (i = 0; i < array_size; i++) { // walk the jobs
       responces++;
       minergate_do_job_rsp* work = mp_rsp->rsp+i;
       if (work->winner_nonce) {
         //printf("!!!GOT minergate job rsp %08x %08x\n",work->work_id_in_sw,work->winner_nonce);
         int job_difficulty = 1<<(leading_zeroes-32); // in 4GH units
         rate += job_difficulty;
       }
    }

   
   //printf("REQUESTS: %d RESPONCES: %d, DELTA: %d\n",requests, responces, requests - responces);
   usleep(jobs_period*1000);
   static int counter = 0;
   counter++;
   // Show rate each X seconds 
   if ((counter%((1000/jobs_period))) == 0) {
      global_rate+=rate;
      global_rate_cnt++;
      printf(MAGENTA "HASH RATE=%dGH (TOTAL=%fGH)\n" RESET,rate*4,((4.0*(float)global_rate)/(float)global_rate_cnt));
      rate=0;
   }
 }
 close(socket_fd);
 free(mp_req);
 return 0;
}

/*

int main(int argc, char *argv[]) {
  minergate_packet *mp = allocate_minergate_packet(1000, 1, 2);
  minergate_data* md1 =  get_minergate_data(mp,  100, 1);
  minergate_data* md2 =  get_minergate_data(mp,  200, 2);
  minergate_data* md3 =  get_minergate_data(mp,  300, 3);
  printf("Got:\n");

  parse_minergate_packet(mp, minergate_data_processor);

  return 0;
}
*/


