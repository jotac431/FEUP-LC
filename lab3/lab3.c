#include <lcom/lcf.h>
#include <lcom/timer.h>
#include <timer.c>

#include <i8042.h>
#include <keyboard.h>
#include <lcom/lab3.h>
#include <stdbool.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

extern uint8_t scancode;
bool two_byte = false;
int counter;

int(kbd_test_scan)() {

  int ipc_status;
  int r;
  message msg;
  uint8_t count = 0;
  uint8_t irq_set;
  uint8_t bytes[2];

  if (kbc_subscribe_int(&irq_set))
    return 1;

  while (scancode != ESQ_BREAKCODE) {
    /* Get a request message */
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                             /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
            kbc_ih();
            count += 2;
            if (!is_two_byte()) {
              if (two_byte) {
                bytes[1] = scancode;
                kbd_print_scancode(is_make(), 2, bytes);
                bytes[0] = 0;
                two_byte = false;
              }
              else {
                bytes[0] = scancode;
                kbd_print_scancode(is_make(), 1, bytes);
              }
            }
            else {
              bytes[0] = scancode;
              two_byte = true;
            }
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
    else { /* received a standard message, not a notification */
           /* no standard messages expected: do nothing */
    }
  }

  kbd_print_no_sysinb(count);

  if (kbc_unsubscribe_int())
    return 1;

  return 0;
}

int(kbd_test_poll)() {

  uint8_t st;
  uint8_t bytes[2];
  int cnt = 0;

  while (scancode != ESQ_BREAKCODE) {
    util_sys_inb(STAT_REG, &st); /* assuming it returns OK */
    cnt++;
    /* loop while 8042 output buffer is empty */
    if ((st & STAT_REG_OBF) && ((st & (STAT_REG_PAR | STAT_REG_TIM | STAT_REG_AUX)) == 0)) {
      util_sys_inb(OUT_BUF, &scancode); /* ass. it returns OK */
      cnt++;

      if (!is_two_byte()) {
        if (two_byte) {
          bytes[1] = scancode;
          kbd_print_scancode(is_make(), 2, bytes);
          bytes[0] = 0;
          two_byte = false;
        }
        else {
          bytes[0] = scancode;
          kbd_print_scancode(is_make(), 1, bytes);
        }
      }
      else {
        bytes[0] = scancode;
        two_byte = true;
      }
    }
    else {                                  // If OBF is not full wait for it
      tickdelay(micros_to_ticks(DELAY_US)); // 5 - Makes keyboard respond to a command in 20 ms
    }
  }

  kbd_print_no_sysinb(cnt);

  if (enable_interrupts())
    return 1;

  return 0;
}

int(kbd_test_timed_scan)(uint8_t n) {

  int ipc_status;
  int r;
  message msg;
  uint8_t count = 0;
  uint8_t keyboard_irq, timer_irq;
  uint8_t bytes[2];

  if (kbc_subscribe_int(&keyboard_irq))
    return 1;

  if (timer_subscribe_int(&timer_irq))
    return 1;

  while (scancode != ESQ_BREAKCODE && (counter / 60.0 < n)) {
    /* Get a request message */
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                                  /* hardware interrupt notification */
          if (msg.m_notify.interrupts & keyboard_irq) { /* subscribed interrupt */
            kbc_ih();
            count += 2;
            if (!is_two_byte()) {
              if (two_byte) {
                bytes[1] = scancode;
                kbd_print_scancode(is_make(), 2, bytes);
                bytes[0] = 0;
                two_byte = false;
              }
              else {
                bytes[0] = scancode;
                kbd_print_scancode(is_make(), 1, bytes);
              }
            }
            else {
              bytes[0] = scancode;
              two_byte = true;
            }
          }

          else if (msg.m_notify.interrupts & timer_irq) { // subscribed interrupt
            // dealing with the interruption call
            timer_int_handler(); // if interruption increment timer
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
    else { /* received a standard message, not a notification */
           /* no standard messages expected: do nothing */
    }
  }

  kbd_print_no_sysinb(count);

  if (kbc_unsubscribe_int())
    return 1;

  if (timer_unsubscribe_int())
    return 1;

  return 0;
}
