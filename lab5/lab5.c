// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include "i8042.h"
#include "keyboard.h"
#include "video.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you

#include "i8042.h"
#include "kbd.h"
#include "video.h"

#define MEGABYTE 1024 * 1024
#define DIRECT_COLOR_MACRO 0x06
#define INDEXED_COLOR_MODE 0x105
#define VIDEO_CARD 0x10

extern uint8_t scancode;
unsigned int xRes, yRes;
extern vbe_mode_info_t vmi_p;
bool two_byte = false;
extern unsigned int counter;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}


int(video_test_init)(uint16_t mode, uint8_t delay) {

  vg_mode(mode);

  sleep(delay);

  return vg_exit();
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color) {

  vg_init1(mode);

  vg_draw_rectangle(x, y, width, height, color);

  tickdelay(micros_to_ticks(2 * 1E6));

  return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {

  uint16_t width, height, bot_width, bot_height;

  int ipc_status;
  int r;
  message msg;
  uint8_t irq_set;
  uint8_t bytes[2];

  if (vg_mode(mode)) {
    return 1;
  }

  width = xRes / no_rectangles;
  height = yRes / no_rectangles;
  bot_width = xRes % no_rectangles;
  bot_height = yRes % no_rectangles;

  for (unsigned int i = 0; i < no_rectangles; i++) {

    if ((i * height) >= (yRes - bot_height))
      break;

    for (unsigned int j = 0; j < no_rectangles; j++) {

      if ((j * width) >= (xRes - bot_width))
        break;

      if (vmi_p.MemoryModel != DIRECT_COLOR_MACRO) {
        vg_rectangle(j * width, i * height, width, height, indexed_color(j, i, step, first, no_rectangles));
      }
      else {
        uint32_t red = R(i, step, first);
        uint32_t green = G(j, step, first);
        uint32_t blue = B(j, i, step, first);
        vg_rectangle(j * width, i * height, width, height, direct_color(red, green, blue));
      }
    }
  }

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
            if (!is_two_byte()) {
              if (two_byte) {
                bytes[1] = scancode;
                bytes[0] = 0;
                two_byte = false;
              }
              else {
                bytes[0] = scancode;
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

  if (kbc_unsubscribe_int())
    return 1;

  return 0;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {

  int ipc_status;
  int r;
  message msg;
  uint8_t irq_set;
  uint8_t bytes[2];

  if (vg_mode(0x105) != 0) {
    return 1;
  }

  xpm_image_t img;
  uint8_t *map;
  map = xpm_load(xpm, XPM_INDEXED, &img);

  xpm_load(xpm, img.type, &img);

  unsigned int img_height = img.height;
  unsigned int img_width = img.width;

  for (unsigned int h = 0; h < img_height; h++) {
    for (unsigned int w = 0; w < img_width; w++) {
      draw_pixel(x + w, y + h, *map);
      map++;
    }
  }

  if (kbc_subscribe_int(&irq_set)) {
    return 1;
  }

  while (scancode != ESQ_BREAKCODE) {

    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) {
            kbc_ih();
            if (!is_two_byte()) {
              if (two_byte) {
                bytes[1] = scancode;
                bytes[0] = 0;
                two_byte = false;
              }
              else {
                bytes[0] = scancode;
              }
            }
            else {
              bytes[0] = scancode;
              two_byte = true;
            }
          }
          break;
        default:
          break;
      }
    }
  }

  if (kbc_unsubscribe_int()) {
    return 1;
  }

  return 0;
}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,
                     int16_t speed, uint8_t fr_rate) {

  uint8_t bit_no = 0;
  int ipc_status;
  int r;
  message msg;
  uint8_t kbd_irq_set = BIT(0);
  uint8_t timer_irq_set = BIT(1);
  uint8_t bytes[2];

  if (vg_mode(0x105) != 0) {
    return 0;
  }

  xpm_image_t img;
  uint8_t *map;
  map = xpm_load(xpm, XPM_INDEXED, &img);

  xpm_load(xpm, img.type, &img);

  unsigned int img_height = img.height;
  unsigned int img_width = img.width;

  for (unsigned int h = 0; h < img_height; h++) {
    for (unsigned int w = 0; w < img_width; w++) {
      draw_pixel(xi + w, yi + h, *map);
      map++;
    }
  }

  int xNew = xi;
  int yNew = yi;
  int frameC = 0;
  int timeF = sys_hz() / fr_rate;

  if (timer_subscribe_int(&bit_no) != 0) {
    return 1;
  }
  if (kbc_subscribe_int(&bit_no) != 0) {
    return 1;
  }

  while (scancode != ESQ_BREAKCODE) {

    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & kbd_irq_set) {
            kbc_ih();
            if (!is_two_byte()) {
              if (two_byte) {
                bytes[1] = scancode;
                bytes[0] = 0;
                two_byte = false;
              }
              else {
                bytes[0] = scancode;
              }
            }
            else {
              bytes[0] = scancode;
              two_byte = true;
            }
          }

          if (msg.m_notify.interrupts & timer_irq_set && (xNew != xf || yNew != yf)) {

            timer_int_handler();

            if (counter % timeF == 0) {
              if (speed > 0) {

                vg_rectangle(xNew, yNew, img_width, img_height, 0);

                if (xi == xf) {

                  if (yi < yf) {
                    if (yNew + speed > yf)
                      yNew = yf;
                    else
                      yNew = yNew + speed;
                  }
                }
                else {

                  if (yNew - speed < yf)
                    yNew = yf;
                  else
                    yNew = yNew - speed;
                }
              }
              else {

                if (xi < xf) {
                  if (xNew + speed > xNew)
                    xNew = yf;
                  else
                    xNew = xNew + speed;
                }
                else {

                  if (xNew - speed < xf)
                    xNew = xf;
                  else
                    xNew = xNew - speed;
                }
              }

              map = xpm_load(xpm, XPM_INDEXED, &img);
              for (unsigned int h = 0; h < img_height; h++) {
                for (unsigned int w = 0; w < img_width; w++) {
                  draw_pixel(xNew + w, yNew + h, *map);
                  map++;
                }
              }
            }
            else {

              frameC++;

              if (frameC % abs(speed) == 0) {

                vg_rectangle(xNew, yNew, img_width, img_height, 0);

                if (xi == xf) {

                  if (yi < yf) {
                    if (yNew + 1 > yf)
                      yNew = yf;
                    else
                      yNew = yNew + 1;
                  }
                  else {
                    if (yNew - 1 < yf)
                      yNew = yf;
                    else
                      yNew = yNew - 1;
                  }
                }
                else {
                  if (xi < xf) {
                    if (xNew + 1 > xf)
                      xNew = xf;
                    else
                      xNew = xNew + 1;
                  }
                  else {
                    if (xNew - speed < xf)
                      xNew = xf;
                    else
                      xNew = xNew - 1;
                  }
                }
                map = xpm_load(xpm, XPM_INDEXED, &img);
                for (unsigned int h = 0; h < img_height; h++) {
                  for (unsigned int w = 0; w < img_width; w++) {
                    draw_pixel(xNew + w, yNew + h, *map);
                    map++;
                  }
                }
              }
            }
            break;
            default:
              break;
          }
      }
    }
  }

  if (timer_unsubscribe_int() != 0) {
    return 1;
  }
  if (kbc_unsubscribe_int() != 0) {
    return 1;
  }
  if (vg_exit() != 0) {
    return 1;
  }

  return 0;
}
