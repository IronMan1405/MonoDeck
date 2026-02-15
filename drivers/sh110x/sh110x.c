#include "sh110x.h"
#include "sh110x_font.h"
#include "platform_config.h"

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>
#include <stdlib.h>

#define SH110X_WIDTH 128
#define SH110X_HEIGHT 64
#define SH110X_PAGES (SH110X_HEIGHT / 8)

#define SH110X_CMD 0x00
#define SH110X_DATA 0x40

static uint8_t framebuffer[SH110X_WIDTH * SH110X_PAGES];

void plotCirclePoints(int xc, int yc, int x, int y);
void swap(int *a, int *b);

static inline void sh110x_write_cmd(uint8_t cmd) {
    uint8_t buf[2] = {SH110X_CMD, cmd};
    i2c_write_blocking(I2C_PORT, DISPLAY_I2C_ADDRESS, buf, 2, false);
}

static inline void sh110x_write_data(uint8_t *data, size_t len) {
    uint8_t buf[len+1];
    buf[0] = 0x40;
    memcpy(&buf[1], data, len);
    i2c_write_blocking(I2C_PORT, DISPLAY_I2C_ADDRESS, buf, len + 1, false);
}

void sh110x_init(void) {
    i2c_init(I2C_PORT, I2C_BAUDRATE);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    sleep_ms(100);

    // Init sequence (SH1106-compatible)
    sh110x_write_cmd(0xAE); // Display OFF
    sh110x_write_cmd(0xD5); sh110x_write_cmd(0x80); // Clock divide
    sh110x_write_cmd(0xA8); sh110x_write_cmd(0x3F); // Multiplex
    sh110x_write_cmd(0xD3); sh110x_write_cmd(0x00); // Display offset
    sh110x_write_cmd(0x40); // Start line = 0
    sh110x_write_cmd(0xAD); sh110x_write_cmd(0x8B); // DC-DC ON
    sh110x_write_cmd(0xA1); // Segment remap
    sh110x_write_cmd(0xC8); // COM scan dec
    sh110x_write_cmd(0xDA); sh110x_write_cmd(0x12); // COM pins
    sh110x_write_cmd(0x81); sh110x_write_cmd(0x7F); // Contrast
    sh110x_write_cmd(0xA4); // Resume RAM
    sh110x_write_cmd(0xA6); // Normal display
    sh110x_write_cmd(0xAF); // Display ON

    sh110x_clear();
    sh110x_update();
}

void sh110x_clear(void) {
    memset(framebuffer, 0x00, sizeof(framebuffer));
}

void sh110x_update(void) {
    for (uint8_t page = 0; page < SH110X_PAGES; page++) {
        sh110x_write_cmd(0xB0 | page);   // Page address
        sh110x_write_cmd(0x02);          // Column low (SH110X offset)
        sh110x_write_cmd(0x10);          // Column high

        sh110x_write_data(
            &framebuffer[page * SH110X_WIDTH],
            SH110X_WIDTH
        );
    }
}

void sh110x_draw_pixel(int x, int y, bool on) {
    if (x >= SH110X_WIDTH || y >= SH110X_HEIGHT) return;
    
    uint16_t index = x + (y / 8) * SH110X_WIDTH;
    uint8_t bit = 1 << (y % 8);

    if (on) framebuffer[index] |= bit;
    else framebuffer[index] &= ~bit;
}

void sh110x_draw_line(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx + dy;

    while (1) {
        sh110x_draw_pixel(x0, y0, true);

        if (x0 == x1 && y0 == y1) {
            break;
        }

        int e2 = 2 * err;

        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void sh110x_draw_fast_hline(int x1, int y, int x2) {
    for (int i = 0; i < (x2-x1); i++) {
        sh110x_draw_pixel(x1 + i, y, true);
    }
}


void sh110x_draw_char(int x, int y, char c, uint8_t scale) {
    if (c < 32 || c > 127) {
        return;
    }

    const uint8_t *glyph = font5x7[c-32];

    for (int col = 0; col < 5; col++) {
        int bits = glyph[col];
        for (int row = 0; row < 7; row++) {
            if (bits & ( 1<< row)){
                for (int dx = 0; dx < scale; dx++) {
                    for (int dy = 0; dy < scale; dy++) {
                        sh110x_draw_pixel(x + col, y + row, true);
                    }
                }
            }
        }
    }
}

void sh110x_draw_text(int x, int y, const char *text, uint8_t scale) {
    int cursor_x = x;
    while (*text) {
        sh110x_draw_char(cursor_x, y, *text, scale);
        cursor_x += 6 * scale; // 5 pixels width and 1 pixel space
        text++;
    }
}


void sh110x_draw_rect(int x, int y, int w, int h) {
    sh110x_draw_line(x, y, x + w - 1, y); // top
    sh110x_draw_line(x, y, x, y + h - 1); // left
    sh110x_draw_line(x + w - 1, y, x + w - 1, y + h - 1); // right
    sh110x_draw_line(x, y + h - 1, x + w - 1, y + h - 1); // bottom
}

void sh110x_fill_rect(int x, int y, int w, int h) {
    for (int col = 0; col < w; col++) {
        for (int row = 0; row < h; row++) {
            sh110x_draw_pixel(x + col, y + row, true);
        }
    }
}


void sh110x_draw_circle(int xc, int yc, int r) {
    int x = 0;
    int y = r;
    int d = 1 - r;

    while (x <= y) {
        plotCirclePoints(xc, yc, x, y);

        if (d < 0) {
            d += 2*x + 3;
        } else {
            d += 2*(x - y) + 5;
            y--;
        }
        x++;
    }
}

void plotCirclePoints(int xc, int yc, int x, int y) {
    sh110x_draw_pixel(xc + x, yc + y, true);
    sh110x_draw_pixel(xc - x, yc + y, true);
    sh110x_draw_pixel(xc + x, yc - y, true);
    sh110x_draw_pixel(xc - x, yc - y, true);
    sh110x_draw_pixel(xc + y, yc + x, true);
    sh110x_draw_pixel(xc - y, yc + x, true);
    sh110x_draw_pixel(xc + y, yc - x, true);
    sh110x_draw_pixel(xc - y, yc - x, true);
}

void sh110x_fill_circle(int xc, int yc, int r) {
    int x = 0;
    int y = r;
    int d = 1 - r;

    while (x <= y) {

        // draw horizontal scanlines

        sh110x_draw_fast_hline(xc - x, yc + y, xc + x);
        sh110x_draw_fast_hline(xc - x, yc - y, xc + x);
        sh110x_draw_fast_hline(xc - y, yc + x, xc + y);
        sh110x_draw_fast_hline(xc - y, yc - x, xc + y);

        if (d < 0) {
            d += 2*x + 3;
        } else {
            d += 2*(x - y) + 5;
            y--;
        }
        x++;
    }
}

void sh110x_draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    sh110x_draw_line(x1, y1, x2, y2);
    sh110x_draw_line(x2, y2, x3, y3);
    sh110x_draw_line(x3, y3, x1, y1);
}

void sh110x_fill_triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    if (y1 > y2) {
        swap(&y1, &y2);
        swap(&x1, &x2);
    }

    if (y2 > y3) {
        swap(&y2, &y3);
        swap(&x2, &x3);
    }

    if (y1 > y2) {
        swap(&y1, &y2);
        swap(&x1, &x2);
    }

    float dx13 = (y3 - y1) ? (float)(x3 - x1) / (y3 - y1) : 0;
    float dx12 = (y2 - y1) ? (float)(x2 - x1) / (y2 - y1) : 0;
    float dx23 = (y3 - y2) ? (float)(x3 - x2) / (y3 - y2) : 0;

    float sx = x1;
    float ex = x1;
    
    //upper half
    for (int y = y1; y <= y2; y++) {
        int xs = (int)sx;
        int xe = (int)ex;

        if (xs > xe) swap(&xs, &xe);

        // sh110x_draw_line(xs, y, xe, y);
        sh110x_draw_fast_hline(xs, y, xe);

        sx += dx13;
        ex += dx12;
    }

    ex = x2;

    //lower half
    for (int y = y2 + 1; y <= y3; y++) {
        int xs = (int)sx;
        int xe = (int)ex;

        if (xs > xe) swap(&xs, &xe);

        // sh110x_draw_line(xs, y, xe, y);
        sh110x_draw_fast_hline(xs, y, xe);

        sx += dx13;
        ex += dx23;
    }

}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void sh110x_draw_bitmap(int x, int y, const uint8_t *bitmap, int w, int h) {
    for (int j = 0; j < h; j++) {

        for (int i = 0; i < w; i++) {

            int byteIndex = i + (j / 8) * w;
            uint8_t bitMask = 1 << (j % 8);

            if (bitmap[byteIndex] & bitMask) {

                int px = x + i;
                int py = y + j;

                if (px < 0 || px >= SH110X_WIDTH ||
                    py < 0 || py >= SH110X_HEIGHT)
                    continue;

                framebuffer[px + (py / 8) * SH110X_WIDTH] |= (1 << (py % 8));
            }
        }
    }
}


void sh110x_command(uint8_t cmd) {
    uint8_t buf[2];
    buf[0] = 0x00;   // control byte: command
    buf[1] = cmd;

    i2c_write_blocking(I2C_PORT, DISPLAY_I2C_ADDRESS, buf, 2, false);
}