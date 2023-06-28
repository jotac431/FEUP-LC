#include <lcom/lcf.h>
#include <stdint.h>
#include <stdbool.h>
#include "i8042.h"
#include "utils.h"

int hook_id = KB_IRQ;
uint8_t scancode = 0;

void (kbc_ih)() {

    uint8_t st;

    util_sys_inb(STAT_REG, &st);

    if(st & STAT_REG_OBF) {
        if((st & (STAT_REG_PAR | STAT_REG_TIM)) == 0) {
            util_sys_inb(OUT_BUF, &scancode);
        }
    }

}

int (kbc_subscribe_int)(uint8_t *bit_no) {
    *bit_no = BIT(hook_id);
    if(sys_irqsetpolicy(KB_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id))
        return 1;
    return 0;
}

int (kbc_unsubscribe_int)() {
    if(sys_irqrmpolicy(&hook_id))
        return 1;
    return 0;
}

bool is_make() {

  uint8_t mask = 0x80;
  return (scancode & mask) == 0;
}

bool is_two_byte() {
  return scancode == TWO_BYTE_SCANCODE;
}

