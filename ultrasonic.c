#include "ultrasonic.h"

#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#include "constants.h"

// Simple polling driver for HC-SR04. Triggers periodically and records the last distance.

static float last_distance_cm = ULTRASONIC_DISTANCE_UNKNOWN;
static uint32_t last_measure_ms = 0;

static bool measure_once(float *out_cm) {
    // Blocking read: fire a pulse, wait for echo, and compute distance.
    // Send 10 us pulse on TRIG.
    gpio_put(ULTRASONIC_TRIG_PIN, 0);
    sleep_us(2);
    gpio_put(ULTRASONIC_TRIG_PIN, 1);
    sleep_us(10);
    gpio_put(ULTRASONIC_TRIG_PIN, 0);

    // Wait for echo to go high.
    absolute_time_t start_wait = get_absolute_time();
    while (!gpio_get(ULTRASONIC_ECHO_PIN)) {
        if (absolute_time_diff_us(start_wait, get_absolute_time()) > ULTRASONIC_ECHO_TIMEOUT_US) {
            return false;
        }
    }

    // Measure echo high duration.
    absolute_time_t echo_start = get_absolute_time();
    while (gpio_get(ULTRASONIC_ECHO_PIN)) {
        if (absolute_time_diff_us(echo_start, get_absolute_time()) > ULTRASONIC_ECHO_TIMEOUT_US) {
            return false;
        }
    }
    absolute_time_t echo_end = get_absolute_time();
    int64_t echo_us = absolute_time_diff_us(echo_start, echo_end);
    if (echo_us <= 0) {
        return false;
    }

    // Speed of sound ~0.0343 cm/us; divide by 2 for the round trip.
    float cm = (float)echo_us * 0.0343f * 0.5f;
    *out_cm = cm;
    return true;
}

void ultrasonic_init(void) {
    gpio_init(ULTRASONIC_TRIG_PIN);
    gpio_set_dir(ULTRASONIC_TRIG_PIN, GPIO_OUT);
    gpio_put(ULTRASONIC_TRIG_PIN, 0);

    gpio_init(ULTRASONIC_ECHO_PIN);
    gpio_set_dir(ULTRASONIC_ECHO_PIN, GPIO_IN);
    gpio_pull_down(ULTRASONIC_ECHO_PIN);  // ensure known idle
}

void ultrasonic_poll(void) {
    static absolute_time_t next_measure = {0};
    absolute_time_t now = get_absolute_time();
    if (!is_nil_time(next_measure)) {
        int64_t until_next = absolute_time_diff_us(now, next_measure);
        if (until_next > 0) {
            return;  // not yet time
        }
    }

    float cm = 0.0f;
    if (measure_once(&cm)) {
        last_distance_cm = cm;
    } else {
        last_distance_cm = ULTRASONIC_DISTANCE_UNKNOWN;
    }
    last_measure_ms = to_ms_since_boot(now);
    next_measure = delayed_by_ms(now, ULTRASONIC_POLL_INTERVAL_MS);
}

float ultrasonic_last_distance_cm(void) {
    return last_distance_cm;
}

uint32_t ultrasonic_last_measure_ms(void) {
    return last_measure_ms;
}
