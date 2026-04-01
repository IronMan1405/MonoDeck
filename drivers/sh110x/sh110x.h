#pragma once

#include <stdint.h>
#include <stdbool.h>

void sh110x_init(void);
void sh110x_clear(void);
void sh110x_update(void);
void sh110x_draw_pixel(int x, int y, bool on);
void sh110x_draw_line(int x0, int y0, int x1, int y1);
void sh110x_draw_char(int x, int y, char c, uint8_t scale);
void sh110x_draw_text(int x, int y, const char *text, uint8_t scale);

void sh110x_draw_rect(int x, int y, int w, int h);
void sh110x_fill_rect(int x, int y, int w, int h);
void sh110x_draw_circle(int xc, int yc, int r);
void sh110x_fill_circle(int xc, int yc, int r);
void sh110x_draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3);
void sh110x_fill_triangle(int x1, int y1, int x2, int y2, int x3, int y3);

void sh110x_draw_bitmap(int x, int y, const uint8_t *bitmap, int w, int h);
void sh110x_command(uint8_t cmd);
void sh110x_dither_fade(int y_start, int height);

