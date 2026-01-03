#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "hardware/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

// Minimal SSD1306 128x64 I2C interface for text rendering.

void ssd1306_init(i2c_inst_t *i2c, uint sda_pin, uint scl_pin, uint8_t addr);
void ssd1306_clear(void);
void ssd1306_draw_text(uint8_t x, uint8_t y, const char *text);
void ssd1306_flush(void);

#ifdef __cplusplus
}
#endif
