#pragma once

#include <lcom/lab3.h>
#include <stdbool.h>
#include <stdint.h>

int(kbc_subscribe_int)(uint8_t *bit_no);

int(kbc_unsubscribe_int)();

bool is_make();

bool is_two_byte();

int enable_interrupts();

int kbc_write_command(uint8_t cmd);

int kbc_read_out_buf(uint8_t *content);

int kbc_write_argument(uint8_t arg);
