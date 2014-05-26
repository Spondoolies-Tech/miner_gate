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



/*
    Minergate server network interface.
    Multithreaded application, listens on socket and handles requests.
    Pushes mining requests from network to lock and returns 
    asynchroniously responces from work_minergate_rsp.

    Each network connection has adaptor structure, that holds all the
    data needed for the connection - last packet with request, 
    next packet with responce etc`.

    
    by Zvisha Shteingart
*/
#include "defines.h"
#include "mg_proto_parser.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h> //for sockets
#include <netinet/in.h> //for internet
#include <pthread.h>
#include "hammer.h"
#include <queue>
#include "spond_debug.h"
#include "squid.h"
#include "i2c.h"
#include "dc2dc.h"
#include "ac2dc.h"
#include "pwm_manager.h"
#include "hammer_lib.h"
#include "miner_gate.h"
#include "scaling_manager.h"
#include "corner_discovery.h"
#include "asic_thermal.h"
#include <syslog.h>
#include "pll.h"
#include "real_time_queue.h"
#include <signal.h>
#include "leds.h"
#include <sys/types.h>
#include <dirent.h>


using namespace std;
pthread_mutex_t network_hw_mutex = PTHREAD_MUTEX_INITIALIZER;
struct sigaction termhandler, inthandler;


// SERVER
void compute_hash(const unsigned char *midstate, unsigned int mrkle_root,
                  unsigned int timestamp, unsigned int difficulty,
                  unsigned int winner_nonce, unsigned char *hash);
int get_leading_zeroes(const unsigned char *hash);
void memprint(const void *m, size_t n);

typedef class {
public:
  uint8_t adapter_id;
  // uint8_t  adapter_id;
  int connection_fd;
  pthread_t conn_pth;
  minergate_req_packet *last_req;
  minergate_rsp_packet *next_rsp;

  // pthread_mutex_t queue_lock;
  queue<minergate_do_job_req> work_minergate_req;
  queue<minergate_do_job_rsp> work_minergate_rsp;

  void minergate_adapter() {}
} minergate_adapter;

minergate_adapter *adapter = NULL;
int kill_app = 0;


extern void save_rate_temp(int back_tmp, int front_tmp, int total_mhash);

void exit_nicely(int seconds_sleep_before_exit) {
  int err;
  kill_app = 1;
  // Let other threads finish. 
  //set_light(LIGHT_YELLOW, LIGHT_MODE_OFF);
  usleep(1000*1000);
  set_light(LIGHT_GREEN, LIGHT_MODE_OFF);   
  disable_engines_all_asics();
  for (int l = 0 ; l < LOOP_COUNT ; l++) {
    dc2dc_disable_dc2dc(l, &err); 
  }
  set_fan_level(0);
  save_rate_temp(0,0,0);
  psyslog("Here comes unexpected death!\n");
  usleep(seconds_sleep_before_exit*1000*1000);
  exit(0);  
}







int read_work_mode(int input_voltage) {
	FILE* file = fopen ("/etc/mg_custom_mode", "r");
  vm.max_ac2dc_power = AC2DC_POWER_LIMIT;
	int i = 0;
  passert(file > 0);
  vm.vmargin_start = true;	
	fscanf (file, "%d %d %d %d %d", &vm.max_fan_level, &vm.voltage_start, &vm.voltage_max, &vm.max_ac2dc_power, &vm.max_dc2dc_current_16s);
  assert(vm.max_fan_level <= 100);
  assert(vm.max_fan_level >= 0);    
  assert(vm.voltage_start <= 790);
  assert(vm.voltage_start >= 555);
  assert(vm.voltage_max   <= 790);
  assert(vm.voltage_max   >= 555);
  assert(vm.voltage_max   >= vm.voltage_start);
  assert(vm.max_ac2dc_power   >= 1000);
  assert(vm.max_ac2dc_power   <= AC2DC_POWER_LIMIT);
  if ((vm.max_dc2dc_current_16s   > 70) || (vm.max_dc2dc_current_16s   < 50)) {
    vm.max_dc2dc_current_16s = 61;
  }
  vm.max_dc2dc_current_16s *= 16;
  fclose (file);

  vm.vtrim_start = VOLTAGE_TO_VTRIM_MILLI(vm.voltage_start);
  vm.vtrim_max = VOLTAGE_TO_VTRIM_MILLI(vm.voltage_max);


  if (input_voltage < 130) {
     psyslog("input_voltage < 130, limit power to 1100\n");
     vm.max_ac2dc_power = 1100;
     if (vm.voltage_start > 640) {
        vm.voltage_start = 640;
     }
  } 


  // compute VTRIM
  psyslog(
    "vm.max_fan_level: %d, vm.voltage_start: %d, vm.voltage_end: %d vm.vtrim_start: %x, vm.vtrim_end: %x\n"
    ,vm.max_fan_level, vm.voltage_start, vm.voltage_max, vm.vtrim_start, vm.vtrim_max); 

	
}




static void sighandler(int sig)
{
  /* Restore signal handlers so we can still quit if kill_work fails */  
  sigaction(SIGTERM, &termhandler, NULL);
  sigaction(SIGINT, &inthandler, NULL);
  exit_nicely(0);
}


void print_adapter(FILE *f ) {
  minergate_adapter *a = adapter;
  if (a) {
    fprintf(f, "Adapter queues: rsp=%d, req=%d\n", 
            a->work_minergate_rsp.size(),
           a->work_minergate_req.size());
  } else {
    fprintf(f, "No Adapter\n");
  }
}

void free_minergate_adapter(minergate_adapter *a) {
  close(a->connection_fd);
  free(a->last_req);
  free(a->next_rsp);
  delete a;
}

int SWAP32(int x) {
  return ((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) |
          (((x) & 0xff0000) >> 8) | (((x) >> 24) & 0xff));
}

//
// Return the winner (or not winner) job back to the addapter queue.
//
void push_work_rsp(RT_JOB *work) {
  pthread_mutex_lock(&network_hw_mutex);
  minergate_do_job_rsp r;
  uint8_t adapter_id = work->adapter_id;
  minergate_adapter *a = adapter;
  if (!a) {
    DBG(DBG_NET, "Adapter disconected! Packet to garbage\n");
    pthread_mutex_unlock(&network_hw_mutex);
    return;
  }
  int i;
  r.mrkle_root = work->mrkle_root;
  r.winner_nonce[0] = work->winner_nonce[0];
  r.winner_nonce[1] = work->winner_nonce[1];  
  r.work_id_in_sw = work->work_id_in_sw;
  r.ntime_offset = work->ntime_offset;
  r.res = 0;
  a->work_minergate_rsp.push(r);
  pthread_mutex_unlock(&network_hw_mutex);
}

//
// returns success, fills W with new job from adapter queue
//
int pull_work_req_adapter(RT_JOB *w, minergate_adapter *a) {
  minergate_do_job_req r;

  if (!a->work_minergate_req.empty()) {
    r = a->work_minergate_req.front();
    a->work_minergate_req.pop();
    w->difficulty = r.difficulty;
    memcpy(w->midstate, r.midstate, sizeof(r.midstate));
    w->adapter_id = a->adapter_id;
    w->mrkle_root = r.mrkle_root;
    w->timestamp = r.timestamp;
    w->winner_nonce[0] = 0;
    w->winner_nonce[1] = 0;    
    w->work_id_in_sw = r.work_id_in_sw;
    w->work_state = 0;
    w->leading_zeroes = r.leading_zeroes;
    w->ntime_max = r.ntime_limit;
    w->ntime_offset = r.ntime_offset;
    return 1;
  }
  return 0;
}

// returns success
int has_work_req_adapter(minergate_adapter *a) {
  return (a->work_minergate_req.size());
}

// returns success
int pull_work_req(RT_JOB *w) {
  // go over adapters...
  pthread_mutex_lock(&network_hw_mutex);
  int ret = false;
  minergate_adapter *a = adapter;
  if (a) {
      ret =pull_work_req_adapter(w, a);
  }
  pthread_mutex_unlock(&network_hw_mutex);
  return ret;
}

int has_work_req() {
  pthread_mutex_lock(&network_hw_mutex);
  minergate_adapter *a = adapter;
  if (a) {
    has_work_req_adapter(a);
  }
  pthread_mutex_unlock(&network_hw_mutex);
}

void push_work_req(minergate_do_job_req *req, minergate_adapter *a) {
  pthread_mutex_lock(&network_hw_mutex);
#if 1
  if (a->work_minergate_req.size() >= (MINERGATE_TOTAL_QUEUE - 10)) {
    minergate_do_job_rsp rsp;
    rsp.mrkle_root = req->mrkle_root;
    rsp.winner_nonce[0] = 0;
    rsp.winner_nonce[1] = 0;    
    rsp.ntime_offset = req->ntime_offset;    
    rsp.work_id_in_sw = req->work_id_in_sw;
    rsp.res = 1;
    // printf("returning %d %d\n",req->work_id_in_sw,rsp.work_id_in_sw);
    a->work_minergate_rsp.push(rsp);
  } else {
    a->work_minergate_req.push(*req); 
  }
#else
  if (a->work_minergate_req.size() >= (MINERGATE_TOTAL_QUEUE - 10)) {
    RT_JOB w;
    pthread_mutex_unlock(&network_hw_mutex);
    pull_work_req(&w);
    push_work_rsp(&w);
    pthread_mutex_lock(&network_hw_mutex);   
  }
  a->work_minergate_req.push(*req); 
#endif
  pthread_mutex_unlock(&network_hw_mutex);
}

// returns success
int pull_work_rsp(minergate_do_job_rsp *r, minergate_adapter *a) {
  pthread_mutex_lock(&network_hw_mutex);
  if (!a->work_minergate_rsp.empty()) {
    *r = a->work_minergate_rsp.front();
    a->work_minergate_rsp.pop();
    pthread_mutex_unlock(&network_hw_mutex);
    return 1;
  }
  pthread_mutex_unlock(&network_hw_mutex);
  return 0;
}
extern pthread_mutex_t hammer_mutex;
//
// Support new minergate client
//
void *connection_handler_thread(void *adptr) {
  psyslog("New adapter connected!\n");
  minergate_adapter *a = (minergate_adapter *)adptr;
  // DBG(DBG_NET,"connection_fd = %d\n", a->connection_fd);
  set_light(LIGHT_GREEN, LIGHT_MODE_FAST_BLINK);
  a->adapter_id = 0;
  a->last_req = allocate_minergate_packet_req(0xca, 0xfe);
  a->next_rsp = allocate_minergate_packet_rsp(0xca, 0xfe);

  vm.idle_probs = 0;
  vm.busy_probs = 0;
  vm.solved_jobs_total = 0;


  RT_JOB work;

  while (one_done_sw_rt_queue(&work)) {
    push_work_rsp(&work);
  }
  int nbytes;

  // Read packet
  struct timeval now;      
  struct timeval last_time; 
  gettimeofday(&now, NULL);
  gettimeofday(&last_time, NULL);
  while ((nbytes = read(a->connection_fd, (void *)a->last_req,
                        sizeof(minergate_req_packet))) > 0) {
    struct timeval now;      
    struct timeval last_time; 
    int usec;
    if (nbytes) {
      passert(a->last_req->protocol_version == MINERGATE_PROTOCOL_VERSION);      
      passert(a->last_req->magic == 0xcaf4);
      gettimeofday(&now, NULL);

      usec = (now.tv_sec - last_time.tv_sec) * 1000000;
      usec += (now.tv_usec - last_time.tv_usec);

      pthread_mutex_lock(&hammer_mutex);     
      pthread_mutex_lock(&network_hw_mutex);
      vm.not_mining_time = 0;
      if (vm.asics_shut_down_powersave) {
        unpause_all_mining_engines();
      }
      pthread_mutex_unlock(&network_hw_mutex);
      pthread_mutex_unlock(&hammer_mutex);

      // Reset packet.
      int i;
      // Return all previous responces
      int rsp_count = a->work_minergate_rsp.size();
      DBG(DBG_NET, "Sending %d minergate_do_job_rsp\n", rsp_count);
      if (rsp_count > MAX_RESPONDS) {
        rsp_count = MAX_RESPONDS;
      }

      for (i = 0; i < rsp_count; i++) {
        minergate_do_job_rsp *rsp = a->next_rsp->rsp + i;
        int res = pull_work_rsp(rsp, a);
        passert(res);
      }
      a->next_rsp->rsp_count = rsp_count;
      int mhashes_done = (vm.total_mhash>>10)*(usec>>10);
      a->next_rsp->gh_div_10_rate = mhashes_done>>10;  
      int array_size = a->last_req->req_count;
      for (i = 0; i < array_size; i++) { // walk the jobs
        minergate_do_job_req *work = a->last_req->req + i;
        push_work_req(work, a);
      }

      if (a->last_req->mask & 0x02) {
         // Drop old requests
         psyslog("----- Drop old requests\n");
         RT_JOB w;
         while (pull_work_req(&w)) {
           // Push to complete queue
           push_work_rsp(&w);
         }
       }


      // parse_minergate_packet(a->last_req, minergate_data_processor,
      // a, a);
      a->next_rsp->request_id = a->last_req->request_id;
      // Send response
      write(a->connection_fd, (void *)a->next_rsp,
            sizeof(minergate_rsp_packet));

      // Clear packet.
      a->next_rsp->rsp_count = 0;
      last_time = now;
    }
  }
  free_minergate_adapter(a);  
  adapter = NULL;  
  set_light(LIGHT_GREEN, LIGHT_MODE_SLOW_BLINK);
  // Clear the real_time_queue from the old packets
  return 0;
}

int init_socket() {
  struct sockaddr_un address;
  int socket_fd, connection_fd;
  socklen_t address_length;
  pid_t child;

  socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    DBG(DBG_NET, "socket() failed\n");
    perror("Err:");
    return 1;
  }

  unlink(MINERGATE_SOCKET_FILE);

  /* start with a clean address structure */
  memset(&address, 0, sizeof(struct sockaddr_un));

  address.sun_family = AF_UNIX;
  sprintf(address.sun_path, MINERGATE_SOCKET_FILE);

  if (bind(socket_fd, (struct sockaddr *)&address,
           sizeof(struct sockaddr_un)) !=
      0) {
    perror("Err:");
    return 0;
  }

  if (listen(socket_fd, 5) != 0) {
    perror("Err:");
    return 0;
  }

  return socket_fd;
}



void test_asic_reset() {
  // Reset ASICs
  write_reg_broadcast(ADDR_GOT_ADDR, 0);

  // If someone not reseted (has address) - we have a problem
  int reg = read_reg_broadcast(ADDR_BR_NO_ADDR);
  if (reg == 0) {
    // Don't remove - used by tests
    printf("got reply from ASIC 0x%x\n", BROADCAST_READ_ADDR(reg));
    printf("RESET BAD\n");
  } else {
    // Don't remove - used by tests
    printf("RESET GOOD\n");
  }
  return;
}


void enable_sinal_handler() {
  struct sigaction handler;
  handler.sa_handler = &sighandler;
  handler.sa_flags = 0;
  sigemptyset(&handler.sa_mask);
  sigaction(SIGTERM, &handler, &termhandler);
  sigaction(SIGINT, &handler, &inthandler);

}
 
void reset_squid() {
  FILE *f = fopen("/sys/class/gpio/export", "w");
  if (!f)
    return;
  fprintf(f, "47");
  fclose(f);
  f = fopen("/sys/class/gpio/gpio47/direction", "w");
  if (!f)
    return;
  fprintf(f, "out");
  fclose(f);
  f = fopen("/sys/class/gpio/gpio47/value", "w");
  if (!f)
    return;
  fprintf(f, "0");
  usleep(10000);
  fprintf(f, "1");
  usleep(20000);
  fclose(f);
}



pid_t proc_find(const char* name) 
{
    DIR* dir;
    struct dirent* ent;
    char buf[512];

    long  pid;
    char pname[100] = {0,};
    char state;
    FILE *fp=NULL; 

    if (!(dir = opendir("/proc"))) {
        perror("can't open /proc");
        return -1;
    }

    while((ent = readdir(dir)) != NULL) {
        long lpid = atol(ent->d_name);
        if(lpid < 0)
            continue;
        snprintf(buf, sizeof(buf), "/proc/%ld/stat", lpid);
        fp = fopen(buf, "r");

        if (fp) {
            if ( (fscanf(fp, "%ld (%[^)]) %c", &pid, pname, &state)) != 3 ){
                printf("fscanf failed \n");
                fclose(fp);
                closedir(dir);
                return -1; 
            }
            if (!strcmp(pname, name)) {
                fclose(fp);
                closedir(dir);
                return (pid_t)lpid;
            }
            fclose(fp);
        }
    }


closedir(dir);
return -1;
}




int main(int argc, char *argv[]) {
  printf(RESET);
  int testreset_mode = 0;
  int init_mode = 0;
  int s;


   if (proc_find("miner_gate_arm") == -1) {
       printf("miner_gate_arm is already running\n");
       exit(0);
   }



  srand (time(NULL));
  enable_reg_debug = 0;
  setlogmask(LOG_UPTO(LOG_INFO));
  openlog("minergate", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
  syslog(LOG_NOTICE, "minergate started");
 
  enable_sinal_handler();

  if ((argc > 1) && strcmp(argv[1], "--help") == 0) {
    printf("--testreset = Test asic reset!!!\n");
    return 0;
  }


  if ((argc > 1) && strcmp(argv[1], "--testreset") == 0) {
    testreset_mode = 1;
    printf("Test reset mode!!!\n");
  }


  struct sockaddr_un address;
  int socket_fd, connection_fd;
  socklen_t address_length = sizeof(address);
  pthread_t main_thread;
  pthread_t dc2dc_thread;
  int input_voltage;
  // pthread_t conn_pth;
  pid_t child;
  psyslog("i2c_init\n");
  i2c_init();
  psyslog("ac2dc_init\n");
  ac2dc_init(&input_voltage);
  psyslog("Read work mode\n");
  FILE* file = fopen ("/etc/voltage", "w");
  if (file > 0) {
    fprintf (file, "%d", input_voltage);	  
    fclose(file);
  }
  printf("Voltage: %d\n", input_voltage);  
  read_work_mode(input_voltage);
  // Must be done after "read_work_mode"
  psyslog("Read  NVM\n");
  load_nvm_ok();
  psyslog("reset_squid\n");
  reset_squid();
  psyslog("init_spi\n");
  init_spi();

  psyslog("dc2dc_init\n");
  dc2dc_init();
 
  
  psyslog("init_pwm\n");
  init_pwm();
  psyslog("set_fan_level\n");
  set_fan_level(vm.max_fan_level);
  //exit(0);
  reset_sw_rt_queue();
  leds_init();
  //set_light(LIGHT_YELLOW, LIGHT_MODE_ON);
  set_light(LIGHT_GREEN, LIGHT_MODE_SLOW_BLINK);


  // test SPI
  int q_status = read_spi(ADDR_SQUID_PONG);
  passert((q_status == 0xDEADBEEF),
          "ERROR: no 0xdeadbeef in squid pong register!\n");



  // Find good loops
  // Update vm.good_loops
  // Set ASICS on all disabled loops to asic_ok=0
  discover_good_loops();

 

  //set loops in FPGA
  if (!enable_good_loops_ok()) {
    psyslog("LOOP TEST FAILED, RESTARTING:%x\n", vm.good_loops);
    passert(0);
  }



  psyslog("enable_good_loops_ok done %d\n", __LINE__);
  // Allocates addresses, sets nonce range.
  // Reset all hammers
  init_hammers();

  
  int addr;
   //passert(read_reg_broadcast(ADDR_VERSION)&0xFF == 0x3c);
 
   while (addr = BROADCAST_READ_ADDR(read_reg_broadcast(ADDR_BR_CONDUCTOR_BUSY))) {
      psyslog(RED "CONDUCTOR BUZY IN %x (%X)\n" RESET, addr,read_reg_broadcast(ADDR_VERSION));
      disable_asic_forever_rt(addr);
   }
    
  // Give addresses to devices.
  allocate_addresses_to_devices(); 
  //passert(read_reg_broadcast(ADDR_VERSION)&0xFF == 0x3c);
  // Set nonce ranges
  set_nonce_range_in_engines(0xFFFFFFFF);

  // Set default frequencies.
  // Set all voltage to ASIC_VOLTAGE_810
  // Set all frequency to ASIC_FREQ_225
  set_safe_voltage_and_frequency();
  // Set all engines to 0x7FFF
  psyslog("hammer initialisation done %d\n", __LINE__);
  thermal_init();
  vm.max_asic_temp = MAX_ASIC_TEMPERATURE;
  

  // Enables NVM engines in ASICs.
  //  printf("enable_voltage_from_nvm\n");
  //  enable_voltage_from_nvm();
    
  psyslog("init_scaling done, ready to mine, saving NVM\n");

  if (testreset_mode) {
    test_asic_reset();
    return 0;
  }


  //compute_corners();
  set_working_voltage_discover_top_speeds();

  
  psyslog("Opening socket for cgminer\n");
  // test HAMMER read
  // passert(read_reg_broadcast(ADDR_VERSION), "No version found in ASICs");
  socket_fd = init_socket();
  passert(socket_fd > 0);

  psyslog("Starting HW thread\n");

  s = pthread_create(&main_thread, NULL, squid_regular_state_machine_rt,
                     (void *)NULL);
  passert(s == 0);
  s = pthread_create(&dc2dc_thread, NULL, i2c_state_machine_nrt,
                     (void *)NULL);
  passert(s == 0);


  adapter = new minergate_adapter;
  passert((int)adapter);
  while ((adapter->connection_fd =
              accept(socket_fd, (struct sockaddr *)&address, &address_length)) >
         -1) {
    // Only 1 thread supportd so far...
    psyslog("New a connected %d %x!\n", adapter->connection_fd, adapter);
    connection_handler_thread((void *)adapter);
    /*
    s = pthread_create(&a->conn_pth, NULL, connection_handler_thread,
                       (void *)a);
    passert(s == 0);
    */
    adapter = new minergate_adapter;
    passert((int)adapter);
  }
  psyslog("Err %d:", adapter->connection_fd);
  passert(0, "Err");

  close(socket_fd);
  unlink(MINERGATE_SOCKET_FILE);
  return 0;
}

