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
#include "hammer.h"
#include "spond_debug.h"
#include <time.h>
#include <syslog.h>
#include <spond_debug.h>
#include "hammer_lib.h"
#include "corner_discovery.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define BROADCAST_ADDR 0xffff

static const char *device = "/dev/spidev1.0";
static uint8_t mode = 0;
static uint8_t bits = 8;
static uint32_t speed = 10000000;
static uint16_t delay = 0;
static int fd = 0;
int assert_serial_failures = true;
int spi_ioctls_read = 0;
int spi_ioctls_write = 0;
int   enable_reg_debug;

#define SPI_CMD_READ 0x01
#define SPI_CMD_WRITE 0x41

typedef struct {
  uint8_t addr;
  uint8_t cmd;
  uint32_t i[64];
} __attribute__((__packed__)) cpi_cmd;



int parse_squid_status(int v) {
  if (v & BIT_STATUS_SERIAL_Q_TX_FULL)
    printf("BIT_STATUS_SERIAL_Q_TX_FULL       ");
  if (v & BIT_STATUS_SERIAL_Q_TX_NOT_EMPTY)
    printf("BIT_STATUS_SERIAL_Q_TX_NOT_EMPTY  ");
  if (v & BIT_STATUS_SERIAL_Q_TX_EMPTY)
    printf("BIT_STATUS_SERIAL_Q_TX_EMPTY      ");
  if (v & BIT_STATUS_SERIAL_Q_RX_FULL)
    printf("BIT_STATUS_SERIAL_Q_RX_FULL       ");
  if (v & BIT_STATUS_SERIAL_Q_RX_NOT_EMPTY)
    printf("BIT_STATUS_SERIAL_Q_RX_NOT_EMPTY  ");
  if (v & BIT_STATUS_SERIAL_Q_RX_EMPTY)
    printf("BIT_STATUS_SERIAL_Q_RX_EMPTY      ");
  if (v & BIT_STATUS_SERVICE_Q_FULL)
    printf("BIT_STATUS_SERVICE_Q_FULL         ");
  if (v & BIT_STATUS_SERVICE_Q_NOT_EMPTY)
    printf("BIT_STATUS_SERVICE_Q_NOT_EMPTY    ");
  if (v & BIT_STATUS_SERVICE_Q_EMPTY)
    printf("BIT_STATUS_SERVICE_Q_EMPTY        ");
  if (v & BIT_STATUS_FIFO_SERIAL_TX_ERR)
    printf("BIT_STATUS_FIFO_SERIAL_TX_ERR     ");
  if (v & BIT_STATUS_FIFO_SERIAL_RX_ERR)
    printf("BIT_STATUS_FIFO_SERIAL_RX_ERR     ");
  if (v & BIT_STATUS_FIFO_SERVICE_ERR)
    printf("BIT_STATUS_FIFO_SERVICE_ERR       ");
  if (v & BIT_STATUS_CHAIN_EMPTY)
    printf("BIT_STATUS_CHAIN_EMPTY            ");
  if (v & BIT_STATUS_LOOP_TIMEOUT_ERROR)
    printf("BIT_STATUS_LOOP_TIMEOUT_ERROR     ");
  if (v & BIT_STATUS_LOOP_CORRUPTION_ERROR)
    printf("BIT_STATUS_LOOP_CORRUPTION_ERROR  ");
  if (v & BIT_STATUS_ILLEGAL_ACCESS)
    printf("BIT_STATUS_ILLEGAL_ACCESS         ");
  printf("\n");
}


void read_spi_mult(uint8_t addr, int count, uint32_t values[]) {
  int ret;
  cpi_cmd tx;
  cpi_cmd rx;

  tx.addr = addr;
  tx.cmd = count;
  passert(count < 16);
  memset(tx.i, 0, 16);
  spi_ioctls_read++;
  struct spi_ioc_transfer tr;
  tr.tx_buf = (unsigned long)&tx;
  tr.rx_buf = (unsigned long)&rx;
  tr.len = 2 + 4 * count;
  tr.delay_usecs = 0;
  tr.speed_hz = speed;
  tr.bits_per_word = bits;

  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  int i;
  for (i = 0; i < count; i++) {
    values[i] = htonl(rx.i[i]);
  }
  if (ret < 1)
    pabort("can't send spi message");

  for (int j = 0 ; j < count ; j++) {
	 //printf("SPI[%x]=>%x\n", addr, values[j]);
  }

  // printf("%02X %02X %08X\n",rx.addr, rx.cmd, rx.i);
}

uint32_t read_spi(uint8_t addr) {
  int ret;
  cpi_cmd tx;
  cpi_cmd rx;

  tx.addr = addr;
  tx.cmd = SPI_CMD_READ;
  tx.i[0] = 0;
  spi_ioctls_read++;

  struct spi_ioc_transfer tr;
  tr.tx_buf = (unsigned long)&tx;
  tr.rx_buf = (unsigned long)&rx;
  tr.len = 2 + 4 * 1;
  tr.delay_usecs = 0;
  tr.speed_hz = speed;
  tr.bits_per_word = bits;

  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  rx.i[0] = htonl(rx.i[0]);
  if (ret < 1)
    pabort("can't send spi message");

  //printf("SPI[%x]=>%x\n", addr,rx.i[0]);
  return rx.i[0];
}

void write_spi_mult(uint8_t addr, int count, int values[]) {
  int ret;
  cpi_cmd tx;
  cpi_cmd rx;
  passert(count < 64);
  spi_ioctls_write++;

  tx.addr = addr;
  tx.cmd = 0x40 | count;
  int i;
  for (i = 0; i < count; i++) {
    tx.i[i] = htonl(values[i]);
  }

  struct spi_ioc_transfer tr;
  tr.tx_buf = (unsigned long)&tx;
  tr.rx_buf = (unsigned long)&rx;
  tr.len = 2 + 4 * count;
  tr.delay_usecs = 0;
  tr.speed_hz = speed;
  tr.bits_per_word = bits;

  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

  if (ret < 1) {
    pabort("can't send spi message");
    passert(0);
  }
  for (int j = 0 ; j < count ; j++) {
  	//printf("SPI[%x]<=%x\n", addr, values[j]);
  }
}

void write_spi(uint8_t addr, uint32_t data) {
  int ret;
  cpi_cmd tx;
  cpi_cmd rx;

  tx.addr = addr;
  tx.cmd = SPI_CMD_WRITE;
  tx.i[0] = htonl(data);
  spi_ioctls_write++;

  struct spi_ioc_transfer tr;
  tr.tx_buf = (unsigned long)&tx;
  tr.rx_buf = (unsigned long)&rx;
  tr.len = 2 + 4 * 1;
  tr.delay_usecs = 0;
  tr.speed_hz = speed;
  tr.bits_per_word = bits;

  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

  if (ret < 1) {
    pabort("can't send spi message");
    passert(0);
  }

  //printf("SPI[%x]<=%x\n", addr, data);
}

void init_spi() {
  int ret = 0;
  fd = open(device, O_RDWR);
  if (fd < 0)
    pabort("can't open spi device. Did you init eeprom?\n");

  /*
  * spi mode
  */
  ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
  if (ret == -1)
    pabort("can't set spi mode");

  ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
  if (ret == -1)
    pabort("can't get spi mode");

  /*
  * bits per word
  */
  ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
  if (ret == -1)
    pabort("can't set bits per word");

  ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
  if (ret == -1)
    pabort("can't get bits per word");

  /*
  * max speed hz
  */
  ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
  if (ret == -1)
    pabort("can't set max speed hz");

  ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
  if (ret == -1)
    pabort("can't get max speed hz");

  /*
  //printf("SPI mode: %d\n", mode);
  printf("bits per word: %d\n", bits);
  printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
  */
}

// Create READ and WRITE queues.
#define MAX_FPGA_CMD_QUEUE 64
typedef struct {
  uint32_t addr;     // SET by SW
  uint32_t offset;   // SET by SW
  uint32_t value;    // SET by SW or FPGA
  uint32_t *p_value; // SET by SW or FPGA
  uint8_t b_read;
} QUEUED_REG_ELEMENT;

QUEUED_REG_ELEMENT cmd_queue[MAX_FPGA_CMD_QUEUE] = { 0 };
int current_cmd_queue_ptr = 0;
int current_cmd_hw_queue_ptr = 0;

void create_serial_pkt(uint32_t *d1, uint32_t *d2, uint8_t reg_offset,
                       uint8_t wr_rd, uint16_t dst_chip_addr, uint32_t reg_val,
                       uint8_t general_bits) {
  *d1 = reg_offset << 24;
  *d1 |= wr_rd << 23;
  *d1 |= dst_chip_addr << 7;
  *d1 |= reg_val >> 26;
  *d2 = reg_val << 6;
  *d2 |= general_bits;
}

int hammer_serial_stack[64] = { 0 };
int hammer_serial_stack_size = 0;

void push_hammer_serial_packet_to_hw(uint32_t d1, uint32_t d2) {
  if(hammer_serial_stack_size >= 64) {
	psyslog("hammer_serial_stack_size = %d\n",hammer_serial_stack_size);
	passert(0);
  }
  //printf("hammer_serial_stack_size=%d\n",hammer_serial_stack_size);
  hammer_serial_stack[hammer_serial_stack_size++] = d1;
  hammer_serial_stack[hammer_serial_stack_size++] = d2;
  if (hammer_serial_stack_size == 60) {
    flush_spi_write();
  }
}

void flush_spi_write() {
  if (hammer_serial_stack_size) {
    write_spi_mult(ADDR_SQUID_SERIAL_WRITE, hammer_serial_stack_size,
                   hammer_serial_stack);
    hammer_serial_stack_size = 0;
  }
}

void push_hammer_read(uint32_t addr, uint32_t offset, uint32_t *p_value) {
  QUEUED_REG_ELEMENT *e = &cmd_queue[current_cmd_queue_ptr++];
  //printf("%x %x %x %x\n",current_cmd_queue_ptr,e->addr,e->offset,e->value);
  passert(current_cmd_queue_ptr < MAX_FPGA_CMD_QUEUE);
  //passert(e->addr == 0);
  //passert(e->offset == 0);
  //passert(e->value == 0);
  e->addr = addr;
  e->offset = offset;
  e->p_value = p_value;	
  e->b_read = 1;

  uint32_t d1;
  uint32_t d2;
  create_serial_pkt(&d1, &d2, offset, 1, addr, 0, GENERAL_BITS_COMPLETION);
  // printf("---> %x %x\n",d1,d2);
  push_hammer_serial_packet_to_hw(d1, d2);
}

void push_hammer_write(uint32_t addr, uint32_t offset, uint32_t value) {
  /*
  QUEUED_REG_ELEMENT *e = &cmd_queue[current_cmd_queue_ptr++];
  passert(current_cmd_queue_ptr <= MAX_FPGA_CMD_QUEUE);
  passert(e->addr == 0);
  passert(e->value == 0);
  passert(e->offset == 0);
  e->addr = addr;
  e->value = value;
  e->offset = offset;
  e->b_read = 0;
 */
  uint32_t d1;
  uint32_t d2;
  create_serial_pkt(&d1, &d2, offset, 0, addr, value,
                    /*GENERAL_BITS_COMPLETION*/ 0);
  // printf("---> %x %x\n",d1,d2);
  push_hammer_serial_packet_to_hw(d1, d2);
  if (enable_reg_debug) { 
    printf("./reg h %x %x %x\n", addr, offset, value);
  }
}

int wait_rx_queue_ready() {
  int loops = 0;
  uint32_t q_status = read_spi(ADDR_SQUID_STATUS);
  // printf("Q STATUS:%x \n", q_status);
  while ((++loops < 500) &&
         ((q_status & BIT_STATUS_SERIAL_Q_RX_NOT_EMPTY) == 0)) {
    usleep(1);
    q_status = read_spi(ADDR_SQUID_STATUS);
  }
  return ((q_status & BIT_STATUS_SERIAL_Q_RX_NOT_EMPTY) != 0);
}

uint32_t _read_reg_actual(uint32_t address, uint32_t offset) {
  // uint32_t d1, d2;
  // printf("-!-read SERIAL: %x %x\n",d1,d2);
  int success = wait_rx_queue_ready();

  if (!success) {
    // TODO  - handle timeout?
    if (assert_serial_failures) {
      printf("FAILED TO READ 0x%x 0x%x\n", address, offset);
#ifdef MINERGATE	  
	  good_loops_fast_test();
#endif
#ifdef DC2DC_CHECK_ON_ERROR
//	  check_for_dc2dc_errors();
	  
#endif
      return 0;
    } else {
      printf("Rx queue timeout.\n");
      return 0;
    }
  }

  uint32_t values[2];
  uint32_t v1, v2;
  read_spi_mult(ADDR_SQUID_SERIAL_READ, 2, values);
  // printf("read SERIAL rsp: %x %x\n",d1,d2);
  uint32_t got_addr = ((values[0] >> 7) & 0xFFFF);
  uint32_t got_offset = ((values[0] >> 24) & 0xFF);
  if ((address != got_addr) || (offset != got_offset)) {
    // Data returned corrupted :(
    if (assert_serial_failures) {
      printf("Data corruption: READ:0x%x 0x%x / GOT:0x%x 0x%x \n", address,
             offset, values[0], values[1]);
#ifdef MINERGATE		  
	   good_loops_fast_test();
#endif
      //passert(0, "29578");
    } else {
      printf("Data corruption: READ:0x%x 0x%x / GOT:0x%x 0x%x \n", address,
             offset, values[0], values[1]);
      return 0;
    }
  }
  passert(offset == got_offset, "");
  v1 = values[0] & 0x3F;
  v2 = values[1];
  uint32_t value =
      ((values[0] & 0x3F) << (32 - 6)) | ((values[1] >> 6) & 0x03FFFFFF);

  if (enable_reg_debug) {
    printf("./reg h %x %x  #=%x\n", address, offset, value);
  }

  return value;
}

#define FPGA_QUEUE_FREE 0
#define FPGA_QUEUE_WORKING 1
#define FPGA_QUEUE_FINISHED 2
int fpga_queue_status() {
  if (current_cmd_queue_ptr == 0) {
    passert(current_cmd_hw_queue_ptr == 0);
    return FPGA_QUEUE_FREE;
  } else if (current_cmd_hw_queue_ptr == current_cmd_queue_ptr) {
    return FPGA_QUEUE_FINISHED;
  } else {
    return FPGA_QUEUE_WORKING;
  }
}

void reset_hammer_queue() {
  //memset(cmd_queue, 0, current_cmd_queue_ptr * sizeof(QUEUED_REG_ELEMENT));
  current_cmd_queue_ptr = 0;
  current_cmd_hw_queue_ptr = 0;
}

void squid_wait_hammer_reads() {
  flush_spi_write();
  // printf("HERE!\n");
  while (fpga_queue_status() == FPGA_QUEUE_WORKING) {
    QUEUED_REG_ELEMENT *e = &cmd_queue[current_cmd_hw_queue_ptr++];
    if (e->b_read) {
      e->value = _read_reg_actual(e->addr, e->offset);
      *(e->p_value) = e->value;
    } else {
      passert(0);
    }

    if (current_cmd_hw_queue_ptr == current_cmd_queue_ptr) {
      // printf("squid_wait_hammer_reads: QUEUE EMPTY\n");
      /*
      uint32_t q_status = read_spi(ADDR_SQUID_STATUS);
      passert((q_status & BIT_STATUS_SERIAL_Q_RX_NOT_EMPTY) == 0, "ERROR: queue
      empty but qstatus busy");
      */
      reset_hammer_queue();
      break;
    }
	//usleep(40);
  }

  return;
}

void write_reg_device(uint16_t addr, uint8_t offset, uint32_t value) {
  push_hammer_write(addr, offset, value);
}

void write_reg_broadcast(uint8_t offset, uint32_t value) {
  push_hammer_write(BROADCAST_ADDR, offset, value);
  // squid_wait_hammer_reads()
}

uint32_t read_reg_device(uint16_t addr, uint8_t offset) {
  uint32_t value;
  push_hammer_read(addr, offset, &value);
  flush_spi_write();
  squid_wait_hammer_reads();
  return value;
}

// Note - first 16 bits are device ID, last are response.
uint32_t read_reg_broadcast(uint8_t offset) {
  uint32_t value;
  push_hammer_read(BROADCAST_ADDR, offset, &value);
  flush_spi_write();
  squid_wait_hammer_reads();
  return value;
}

// Note - first 16 bits are device ID, last are response.
uint32_t read_reg_broadcast_test(uint8_t offset) {
  uint32_t value;
  push_hammer_read(BROADCAST_ADDR, offset, &value);
  flush_spi_write();
  // squid_wait_hammer_reads();
  return value;
}
