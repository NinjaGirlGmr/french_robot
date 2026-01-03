#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Configure TRIG/ECHO GPIOs; call once at startup.
void ultrasonic_init(void);

// Trigger a measurement when its interval elapses; call from the main loop.
void ultrasonic_poll(void);

// Most recent distance in centimeters, or ULTRASONIC_DISTANCE_UNKNOWN if invalid.
float ultrasonic_last_distance_cm(void);

// Timestamp (ms since boot) of the last measurement, or 0 if none.
uint32_t ultrasonic_last_measure_ms(void);

#ifdef __cplusplus
}
#endif
