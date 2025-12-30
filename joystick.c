#include "joystick.h"

#include "hardware/adc.h"
#include "pico/stdlib.h"

#include "constants.h"

static uint joystick_x_channel(void) { return JOYSTICK_X_PIN - 26; }
static uint joystick_y_channel(void) { return JOYSTICK_Y_PIN - 26; }

static float center_x = 2047.5f;
static float center_y = 2047.5f;

static float normalize(uint16_t raw, float center) {
    // Convert 12-bit ADC reading to normalized axis in [-1, 1] relative to measured center.
    float span = (raw >= center) ? (4095.0f - center) : center;
    if (span < 1.0f) {
        span = 1.0f;
    }

    float centered = ((float)raw - center) / span;
    if (centered > 1.0f) centered = 1.0f;
    if (centered < -1.0f) centered = -1.0f;

    const float deadzone = 0.08f;
    if (centered > -deadzone && centered < deadzone) {
        centered = 0.0f;
    }
    return centered;
}

void joystick_init(void) {
    // Configure ADC for the X/Y potentiometers and input with pull-up for the button.
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    gpio_init(JOYSTICK_BTN_PIN);
    gpio_set_dir(JOYSTICK_BTN_PIN, GPIO_IN);
    gpio_pull_up(JOYSTICK_BTN_PIN);  // typical hat uses active-low switch

    // Capture resting center positions to reduce drift.
    float sum_x = 0.0f;
    float sum_y = 0.0f;
    const int samples = 16;
    for (int i = 0; i < samples; i++) {
        adc_select_input(joystick_x_channel());
        sum_x += (float)adc_read();
        adc_select_input(joystick_y_channel());
        sum_y += (float)adc_read();
        sleep_us(200);
    }
    center_x = sum_x / samples;
    center_y = sum_y / samples;
}

joystick_state_t joystick_read(void) {
    joystick_state_t state;

    adc_select_input(joystick_x_channel());
    state.raw_x = adc_read();
    adc_select_input(joystick_y_channel());
    state.raw_y = adc_read();

    state.x = normalize(state.raw_x, center_x);
    state.y = normalize(state.raw_y, center_y);
    state.pressed = (gpio_get(JOYSTICK_BTN_PIN) == 0);
    return state;
}
