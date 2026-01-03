#include "lidar.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#include "constants.h"

// Parse 0xFA-prefixed 42-byte frames from common LDS-style lidars.
// Maintains a per-degree distance buffer for quick sector queries.

enum {
    kLdsHeader = 0xFA,
    kLdsFrameSize = 42,
    kLdsPointsPerFrame = 6,
    kLdsPointStride = 6,
    kMaxReasonableDistanceMm = 8000
};

// Forward sector definition (angles measured at the lidar, clockwise).
static const int kFrontCenterDeg = LIDAR_FRONT_OFFSET_DEG;
static const int kFrontHalfSpanDeg = LIDAR_FRONT_HALF_SPAN_DEG;

// Parser state.
static uint8_t frame_buf[kLdsFrameSize];
static size_t frame_pos = 0;
static uint32_t last_byte_ms = 0;

// Distance buffer.
static uint16_t distances[360];
static uint32_t last_scan_ms = 0;
static bool initialized = false;

static inline uint32_t now_ms(void) { return to_ms_since_boot(get_absolute_time()); }

static int wrap_angle(int deg) {
    deg %= 360;
    if (deg < 0) deg += 360;
    return deg;
}

static void reset_if_stale(uint32_t now) {
    // Drop a partial frame if bytes stalled for too long.
    if (frame_pos > 0 && (uint32_t)(now - last_byte_ms) > 50) {
        frame_pos = 0;
    }
}

static void record_point(int angle_deg, uint16_t dist_mm) {
    if (angle_deg < 0 || angle_deg >= 360) {
        return;
    }
    if (dist_mm == 0 || dist_mm > kMaxReasonableDistanceMm) {
        dist_mm = LIDAR_DISTANCE_UNKNOWN;
    }
    distances[angle_deg] = dist_mm;
    last_scan_ms = now_ms();
}

static void parse_frame(void) {
    const uint8_t idx = frame_buf[1];
    if (idx < 0xA0 || idx > 0xDB) {
        return;
    }
    const int base_angle_deg = ((int)idx - 0xA0) * 6;
    for (size_t p = 0; p < kLdsPointsPerFrame; ++p) {
        const size_t off = 4 + p * kLdsPointStride;
        const uint16_t dist_raw = (uint16_t)frame_buf[off] | ((uint16_t)frame_buf[off + 1] << 8);
        const uint16_t dist_mm = dist_raw & 0x3FFF;  // upper bits sometimes carry flags
        const int angle = base_angle_deg + (int)p;
        record_point(angle, dist_mm);
    }
}

void lidar_init(void) {
    if (initialized) {
        return;
    }

    // Set UART pins and format.
    uart_init(LIDAR_UART_ID, LIDAR_UART_BAUD);
    gpio_set_function(LIDAR_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(LIDAR_UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(LIDAR_UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(LIDAR_UART_ID, true);

    for (size_t i = 0; i < 360; i++) {
        distances[i] = LIDAR_DISTANCE_UNKNOWN;
    }
    initialized = true;
}

void lidar_poll(void) {
    if (!initialized) {
        return;
    }

    const uint32_t start_ms = now_ms();
    reset_if_stale(start_ms);

    while (uart_is_readable(LIDAR_UART_ID)) {
        const uint8_t b = uart_getc(LIDAR_UART_ID);
        last_byte_ms = now_ms();

        if (frame_pos == 0) {
            if (b == kLdsHeader) {
                frame_buf[frame_pos++] = b;
            }
            continue;
        }

        frame_buf[frame_pos++] = b;
        if (frame_pos >= kLdsFrameSize) {
            parse_frame();
            frame_pos = 0;
        }
    }
}

bool lidar_has_scan(void) {
    return last_scan_ms != 0;
}

uint16_t lidar_min_distance_in_sector(int center_deg, int half_span_deg) {
    if (!lidar_has_scan()) {
        return LIDAR_DISTANCE_UNKNOWN;
    }
    uint16_t min_mm = LIDAR_DISTANCE_UNKNOWN;
    const int start = wrap_angle(center_deg - half_span_deg);
    const int end = wrap_angle(center_deg + half_span_deg);

    int angle = start;
    while (true) {
        uint16_t d = distances[angle];
        if (d != LIDAR_DISTANCE_UNKNOWN && d < min_mm) {
            min_mm = d;
        }
        if (angle == end) {
            break;
        }
        angle = wrap_angle(angle + 1);
    }
    return min_mm;
}

uint16_t lidar_front_min_distance_mm(void) {
    return lidar_min_distance_in_sector(kFrontCenterDeg, kFrontHalfSpanDeg);
}

uint32_t lidar_last_byte_ms(void) {
    return last_byte_ms;
}
