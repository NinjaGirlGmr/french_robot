#pragma once

// Central pin/function constants for the project.

// L298N motor driver pins (adjust to match wiring).
#define MOTOR_A_EN_PIN 2  // ENA (PWM)
#define MOTOR_A_IN1_PIN 3
#define MOTOR_A_IN2_PIN 4

#define MOTOR_B_EN_PIN 5  // ENB (PWM)
#define MOTOR_B_IN3_PIN 6
#define MOTOR_B_IN4_PIN 7

// Motor supply characteristics.
#define MOTOR_SUPPLY_VOLTAGE 12.0f
#define MOTOR_MIN_VOLTAGE 3.0f
#define MOTOR_MAX_VOLTAGE 12.0f

// Geometry
#define WHEEL_DIAMETER_MM 60.0f
#define WHEEL_DIAMETER_M (WHEEL_DIAMETER_MM / 1000.0f)
#define WHEEL_CIRCUMFERENCE_M (WHEEL_DIAMETER_M * 3.1415926535f)  // distance per wheel revolution
#define TRACK_WIDTH_MM 15.0f
#define TRACK_WIDTH_M (TRACK_WIDTH_MM / 1000.0f)

// Joystick hat (analog) pins.
#define JOYSTICK_X_PIN 26  // ADC0
#define JOYSTICK_Y_PIN 27  // ADC1
#define JOYSTICK_BTN_PIN 22  // Digital (active low if using typical hat)

// PWM settings for motor speed control.
#define PWM_TARGET_HZ 20000.0f
#define PWM_WRAP 999
#define PWM_CLKDIV (125000000.0f / ((PWM_WRAP + 1) * PWM_TARGET_HZ))

// Network control (robot listens for UDP drive commands).
#define CONTROL_UDP_PORT 42424
#define CONTROL_IDLE_TIMEOUT_MS 5000  // coast motors if no command arrives within this window
#define CONTROL_MESSAGE_MAX_LEN 128
