#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    float x;       // normalized [-1, 1]
    float y;       // normalized [-1, 1]
    bool pressed;  // true when button is pressed (assumes active low)
    uint16_t raw_x;
    uint16_t raw_y;
} joystick_state_t;

void joystick_init(void);
joystick_state_t joystick_read(void);
