#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Distance values of this magnitude mean "no reading yet".
#define LIDAR_DISTANCE_UNKNOWN UINT16_MAX

// Initialize the UART-connected lidar sensor.
void lidar_init(void);

// Pump the UART parser; call frequently from the main loop.
void lidar_poll(void);

// True after at least one complete 360-degree sweep has been observed.
bool lidar_has_scan(void);

// Min distance in millimeters within a sector centered at center_deg (degrees) with half-span.
// Returns LIDAR_DISTANCE_UNKNOWN if no data has been received for the sector yet.
uint16_t lidar_min_distance_in_sector(int center_deg, int half_span_deg);

// Convenience helper for the forward sector defined in constants.h.
uint16_t lidar_front_min_distance_mm(void);

// Timestamp of the last ingested lidar byte (ms since boot), or 0 if none.
uint32_t lidar_last_byte_ms(void);

#ifdef __cplusplus
}
#endif
