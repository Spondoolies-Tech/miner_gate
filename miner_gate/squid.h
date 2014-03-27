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

#ifndef _____SPILIB_H____
#define _____SPILIB_H____

#include <stdint.h>
#include <unistd.h>

#define ADDR_SQUID_REVISION 0x0
#define ADDR_SQUID_STATUS 0x1
#define ADDR_SQUID_ISR 0x2
#define ADDR_SQUID_IMR 0x3
#define ADDR_SQUID_ICR 0x4
#define ADDR_SQUID_ITR 0x5
#define ADDR_SQUID_PONG 0xf          // 0xDEADBEEF
#define ADDR_SQUID_LOOP_RESET 0x10   // Each bit = a loop
#define ADDR_SQUID_LOOP_BYPASS 0x11  // Each bit = a loop
#define ADDR_SQUID_LOOP_TIMEOUT 0x12 // Each bit = a loop
#define ADDR_SQUID_SCRATCHPAD 0x1f
#define ADDR_SQUID_COMMAND 0x20
#define ADDR_SQUID_QUEUE_STATUS 0x21
#define ADDR_SQUID_SERVICE_STATUS 0x22
#define ADDR_SQUID_SR_CONTROL_BASE 0x30
#define ADDR_SQUID_SR_MEMORY_BASE 0x40
#define ADDR_SQUID_SERIAL_WRITE 0x80
#define ADDR_SQUID_SERIAL_READ 0x81
#define ADDR_SQUID_SERVICE_READ 0x82

// ADDR_SQUID_STATUS, ADDR_SQUID_ISR, ADDR_SQUID_IMR, ADDR_SQUID_ICR,
// ADDR_SQUID_ITR
#define BIT_STATUS_SERIAL_Q_TX_FULL 0x0001
#define BIT_STATUS_SERIAL_Q_TX_NOT_EMPTY 0x0002
#define BIT_STATUS_SERIAL_Q_TX_EMPTY 0x0004
#define BIT_STATUS_SERIAL_Q_RX_FULL 0x0008
#define BIT_STATUS_SERIAL_Q_RX_NOT_EMPTY 0x0010
#define BIT_STATUS_SERIAL_Q_RX_EMPTY 0x0020
#define BIT_STATUS_SERVICE_Q_FULL 0x0040
#define BIT_STATUS_SERVICE_Q_NOT_EMPTY 0x0080
#define BIT_STATUS_SERVICE_Q_EMPTY 0x0100
#define BIT_STATUS_FIFO_SERIAL_TX_ERR 0x0200
#define BIT_STATUS_FIFO_SERIAL_RX_ERR 0x0400
#define BIT_STATUS_FIFO_SERVICE_ERR 0x0800
#define BIT_STATUS_CHAIN_EMPTY 0x1000
#define BIT_STATUS_LOOP_TIMEOUT_ERROR 0x2000
#define BIT_STATUS_LOOP_CORRUPTION_ERROR 0x4000
#define BIT_STATUS_ILLEGAL_ACCESS 0x8000

// ADDR_SQUID_COMMAND
#define BIT_COMMAND_SERIAL_Q_TX_FIFO_RESET 0x01
#define BIT_COMMAND_SERIAL_Q_RX_FIFO_RESET 0x02
#define BIT_COMMAND_SERVICE_Q_RX_FIFO_RESET 0x04
#define BIT_COMMAND_LOOP_LOGIC_RESET 0x08
#define BIT_COMMAND_SERVICE_ROUTINE_ENABLE 0x10

// ADDR_SQUID_QUEUE_STATUS - offsets!
#define BIT_QUEUE_STATUS_SERIAL_TX_FIFO_USED_OFFSET 0
#define BIT_QUEUE_STATUS_SERIAL_RX_FIFO_USED_OFFSET 8
#define BIT_QUEUE_STATUS_SERVICE_RX_FIFO_USED_OFFSET 16

// ADDR_SQUID_SR_CONTROL
#define BIT_SERVICE_ROUTINE_COUNTER_OFFSET 0
#define BIT_SERVICE_ROUTINE_ENABLE 0x80000000

#define GENERAL_BITS_COMPLETION 0x20
#define GENERAL_BITS_CONDITION 0x10

void write_spi(uint8_t addr, uint32_t data);
uint32_t read_spi(uint8_t addr);
void write_spi_mult(uint8_t addr, int count, int values[]);
void read_spi_mult(uint8_t addr, int count, int values[]);
void squid_wait_hammer_reads();

void init_spi();

void write_hammer_device(uint32_t cpu, uint32_t offset, uint32_t value);
void write_hammer_broadcast(uint32_t offset, uint32_t value);
void read_hammer_device(uint32_t cpu, uint32_t offset, uint32_t *p_value);
void read_hammer_broadcast(uint32_t offset, uint32_t *p_value);

// Flush write command to SPI
void flush_spi_write();
void write_reg_device(uint16_t cpu, uint8_t offset, uint32_t value);
void write_reg_broadcast(uint8_t offset, uint32_t value);
uint32_t read_reg_device(uint16_t cpu, uint8_t offset);
uint32_t read_reg_broadcast(uint8_t offset);
#define BROADCAST_READ_ADDR(X) ((X >> 16) & 0xFF)
#define BROADCAST_READ_DATA(X) (X & 0xFFFF)
#define ASIC_GET_LOOP_OFFSET(ADDR, LOOP, OFFSET)                               \
  {                                                                            \
    LOOP = (ADDR / HAMMERS_PER_LOOP);                                          \
    OFFSET = ADDR % HAMMERS_PER_LOOP;                                          \
  }
#define ASIC_GET_BY_ADDR(ADDR) (&vm.hammer[ADDR])
extern int enable_reg_debug;

#endif
