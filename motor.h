#pragma once

#include <stdbool.h>

typedef enum {
    MOTOR_A = 0,
    MOTOR_B = 1,
} motor_id_t;

void motor_init(void);
void motor_drive(motor_id_t motor, float speed);  // speed in range [-1, 1]
void motor_drive_voltage(motor_id_t motor, float voltage, float supply_voltage);
void motor_brake(motor_id_t motor);
void motor_coast(motor_id_t motor);
void motor_brake_all(void);
void motor_coast_all(void);
