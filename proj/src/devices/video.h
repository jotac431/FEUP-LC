#pragma once

#include <lcom/lcf.h>

int vg_mode(uint16_t mode);

void *vg_init1(uint16_t mode);

int draw_pixel(uint16_t x, uint16_t y, uint32_t color);

int draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

uint8_t* loadXPM(xpm_map_t xpm, enum xpm_image_type type, xpm_image_t *img);

void print_xpm(uint16_t x, uint16_t y,  uint8_t* map, xpm_image_t *img);

void draw_background(char const *xpm[]);

void draw_object(uint16_t x, uint16_t y, uint8_t *ballMap, xpm_image_t ballImg);

void draw_hline(uint16_t x, uint16_t y, uint16_t width, uint32_t color);

void draw_vline(uint16_t x, uint16_t y, uint16_t height, uint32_t color);

void draw_border(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

void draw_black();
