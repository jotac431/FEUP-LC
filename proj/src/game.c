#include <lcom/lcf.h>

#include "game.h"

// Variáveis globais

uint8_t irq_kbd;
uint8_t irq_timer;

int counter;
extern uint8_t scancode;
extern vbe_mode_info_t vmi_p;
bool two_byte = false;
ball_t b;
paddle_t p[2];
int score[] = {0, 0};
extern int width;
extern int height;
uint8_t *ballMap;

// Função de inicialização do jogo

int play() {

  vg_init1(0x14c);

  p[0].x = 0;
  p[0].y = 382;
  p[1].x = 1132;
  p[1].y = 382;

  b.x = width / 2;
  b.y = height / 2;
  b.dx = 5;
  b.dy = 5;

  ballMap = xpm_load(ball, XPM_8_8_8, &b.sprite);

  draw_paddles();

  int ipc_status;
  int r;
  message msg;
  uint8_t bytes[2];

  if (timer_subscribe_int(&irq_timer))
    return 1;

  if (kbc_subscribe_int(&irq_kbd)) {
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
          if (msg.m_notify.interrupts & irq_kbd) {
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

            // draw_border(width/10, height/10, width-width/5, height-height/5, 0xFFFFFF);
            move_paddle_arrows();
            move_paddle_ws();
            draw_paddles();
          }
          if (msg.m_notify.interrupts & irq_timer) {
            timer_int_handler();
            if (counter % 2 == 0) {
              move_ball();
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

  if (timer_unsubscribe_int())
    return 1;

  return vg_exit();
}

// Função que verifica a colisão entre a bola e barra do jogador

int check_collision(ball_t b, paddle_t p) {

  int top_b, top_p; 
  int bottom_b, bottom_p;
  int left_b, left_p;
  int right_b, right_p;

  top_b = b.y;
  bottom_b = b.y + b.sprite.height;
  left_b = b.x;
  right_b = b.x + b.sprite.width;

  top_p = p.y;
  bottom_p = p.y + 100 /*p.sprite.height*/;
  left_p = p.x;
  right_p = p.x + 20 /*p.sprite.width*/;

  if (left_b > right_p)
    return 0;

  if (right_b < left_p)
    return 0;

  if (top_b > bottom_p)
    return 0;

  if (bottom_b < top_p)
    return 0;

  return 1;
}

// Função que move a bola de acordo com o seu vetor de movimento

void move_ball() {

  draw_rectangle(b.x, b.y, b.sprite.width, b.sprite.height, 0);

  b.x += b.dx;
  b.y += b.dy;

  if (b.x < -51) {
    score[1] += 1;
    b.x = width / 2;
    b.y = height / 2;
  }

  if (b.x > width+30) {
    score[0] += 1;
    b.x = width / 2;
    b.y = height / 2;
  }

  if (b.y < 51 || b.y > height - 100) {
    b.dy = -b.dy;
  }

  if (b.x >= 0 && b.x <= width - 20 && b.y >= 0 && b.y <= height - 100) {
    draw_object(b.x, b.y, ballMap, b.sprite);
  }

  for (int i = 0; i < 2; i++) {

    int c = check_collision(b, p[i]);

    // Se for detetada colisão
    if (c == 1) {

      if (b.dx < 0) {
        b.dx -= 1;
      }
      else {
        b.dx += 1;
      }

      // Mudança de direção
      b.dx = -b.dx;

      // Mudar a direção da bola de acordo com o ângulo com que colide com a barra
      int ang_pos = (p[i].y + p[i].sprite.height) - b.y;

      if (ang_pos >= 0 && ang_pos < 7)
        b.dy = 4;

      if (ang_pos >= 7 && ang_pos < 14)
        b.dy = 3;

      if (ang_pos >= 14 && ang_pos < 21)
        b.dy = 2;

      if (ang_pos >= 21 && ang_pos < 28)
        b.dy = 1;

      if (ang_pos >= 28 && ang_pos < 32)
        b.dy = 0;

      if (ang_pos >= 32 && ang_pos < 39)
        b.dy = -1;

      if (ang_pos >= 39 && ang_pos < 46)
        b.dy = -2;

      if (ang_pos >= 46 && ang_pos < 53)
        b.dy = -3;

      if (ang_pos >= 53 && ang_pos < 60)
        b.dy = -4;

      if (b.dx > 0) {
        if (b.x < 30) {
          b.x = 30;
        }
      }
      else {
        if (b.x > 600) {
          //b.x = 600;
        }
      }
    }
  }
}

// Função que permite movimentar a barra da direita ao usar as arrow keys

int move_paddle_arrows() {

  if (scancode == UPARROW_MAKECODE) {

    if (p[1].y <= 20) {
      draw_rectangle(p[1].x, 0, 20, 20, 0);
      p[1].y = 20;
    }
    else {
      draw_rectangle(p[1].x, p[1].y + 80, 20, 20, 0);
      p[1].y -= 20;
    }
  }
  else if (scancode == DOWNARROW_MAKECODE) {

    if (p[1].y >= height - 100 /*p[1].sprite.height*/) {
      draw_rectangle(p[1].x, height - 80, 20, 20, 0);
      p[1].y = height - 100 /*p[1].sprite.height*/;
    }
    else {
      draw_rectangle(p[1].x, p[1].y, 20, 20, 0);
      p[1].y += 20;
    }
  }

  return 0;
}

// Função que permite movimentar a barra da direita ao usar as W e S keys

int move_paddle_ws() {

  if (scancode == W_MAKECODE) {

    if (p[0].y <= 20) {
      draw_rectangle(p[0].x, 0, 20, 20, 0);
      p[0].y = 20;
    }
    else {
      draw_rectangle(p[0].x, p[0].y + 80, 20, 20, 0);
      p[0].y -= 20;
    }
  }
  else if (scancode == S_MAKECODE) {

    if (p[0].y >= height - 120 /*p[0].sprite.height*/) {
      draw_rectangle(p[0].x, height - 20, 20, 20, 0);
      p[0].y = height - 120 /*p[0].sprite.height*/;
    }
    else {
      draw_rectangle(p[0].x, p[0].y, 20, 20, 0);
      p[0].y += 20;
    }
  }

  return 0;
}

void draw_paddles() {
  draw_rectangle(p[0].x, p[0].y, 20, 100, 0xffffff);
  draw_rectangle(p[1].x, p[1].y, 20, 100, 0xffffff);
}
