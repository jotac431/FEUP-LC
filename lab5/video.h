#include <lcom/lcf.h>

#include <machine/int86.h>

int vg_mode(uint16_t mode);
int *vg_init1(uint16_t mode);
int draw_pixel(uint16_t x, uint16_t y, uint32_t color);
int vg_hline(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
int vg_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);
uint32_t (R_First)(uint32_t first);
uint32_t (G_First)(uint32_t first);
uint32_t (B_First)(uint32_t first);
uint32_t (R)(unsigned int w, uint8_t step, uint8_t first);
uint32_t (G)(unsigned int w, uint8_t step, uint8_t first);
uint32_t (B)(unsigned int w, unsigned int h, uint8_t step, uint8_t first);
uint32_t (indexed_color)(uint16_t row, uint16_t col, uint8_t step, uint32_t first, uint8_t no_rectangles);
uint32_t (direct_color)(uint32_t red, uint32_t green, uint32_t blue);
