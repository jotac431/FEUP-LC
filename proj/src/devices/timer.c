#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

int counter = 0;
int hook_timer = 0;

int(timer_set_frequency)(uint8_t timer, uint32_t freq) {

  if (freq > TIMER_FREQ)
    return 1;

  uint16_t val = TIMER_FREQ / freq;

  uint8_t lsb = 0;
  uint8_t msb = 0;

  util_get_LSB(val, &lsb);
  util_get_MSB(val, &msb);

  uint8_t st = 0;
  if (timer_get_conf(timer, &st))
    return 1;

  st &= 0x0F;
  st |= TIMER_LSB_MSB;

  switch (timer) {
    case 0:
      st |= TIMER_SEL0;
      break;
    case 1:
      st |= TIMER_SEL1;
      break;
    case 2:
      st |= TIMER_SEL2;
      break;
  }

  return (sys_outb(TIMER_CTRL, st) || sys_outb(TIMER_0 + timer, lsb) || sys_outb(TIMER_0 + timer, msb));
}

int(timer_subscribe_int)(uint8_t *bit_no) {
  *bit_no = BIT(hook_timer);
  if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_timer))
    return 1;
  return 0;
}

int(timer_unsubscribe_int)() {
  if (sys_irqrmpolicy(&hook_timer))
    return 1;
  return 0;
}

void(timer_int_handler)() {
  counter++;
}

int(timer_get_conf)(uint8_t timer, uint8_t *st) {

  if (timer < 0 || timer > 2)
    return 1;

  uint8_t rbcommand = (TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer));

  if (sys_outb(TIMER_CTRL, rbcommand))
    return 1;

  return util_sys_inb(TIMER_0 + timer, st);
}

int(timer_display_conf)(uint8_t timer, uint8_t st, enum timer_status_field field) {

  union timer_status_field_val conf;
  uint8_t mask = 0x00;

  switch (field) {

    case tsf_all:
      conf.byte = st;
      break;

    case tsf_initial:
      mask = 0x30;
      mask = mask & st;
      mask = mask >> 4;
      switch (mask) {
        case 0x00:
          conf.in_mode = INVAL_val;
          break;
        case 0x01:
          conf.in_mode = LSB_only;
          break;
        case 0x02:
          conf.in_mode = MSB_only;
          break;
        case 0x03:
          conf.in_mode = MSB_after_LSB;
          break;
      }
      break;

    case tsf_mode:
      mask = 0x0E;
      mask = mask & st;
      mask = mask >> 1;
      conf.count_mode = mask;
      if (mask == 0x06 || mask == 0x07)
        conf.count_mode = mask - 0x04;
      break;

    case tsf_base:
      mask = 0x01;
      mask = mask & st;
      if (mask == 0x00)
        conf.bcd = false;
      else
        conf.bcd = true;
      break;

    default:
      break;
  }

  return timer_print_config(timer, field, conf);
}
