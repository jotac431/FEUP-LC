#include <lcom/lcf.h>
#include <stdint.h>
#include <stdbool.h>
#include "i8042.h"

void (kbc_ih)();
int (kbc_subscribe_int)(uint8_t *bit_no);
int (kbc_unsubscribe_int)();
bool is_make();
bool is_two_byte();
