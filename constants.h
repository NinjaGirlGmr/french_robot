#pragma once

// Central pin/function constants for the project.

// L298N motor driver pins (adjust to match wiring).
#define MOTOR_A_EN_PIN 2 // ENA (PWM)
#define MOTOR_A_IN1_PIN 3
#define MOTOR_A_IN2_PIN 4

#define MOTOR_B_EN_PIN 5 // ENB (PWM)
#define MOTOR_B_IN3_PIN 6
#define MOTOR_B_IN4_PIN 7

// Motor supply characteristics.
#define MOTOR_SUPPLY_VOLTAGE 12.0f
#define MOTOR_MIN_VOLTAGE 3.0f
#define MOTOR_MAX_VOLTAGE 12.0f

// Geometry
#define WHEEL_DIAMETER_MM 60.0f
#define WHEEL_DIAMETER_M (WHEEL_DIAMETER_MM / 1000.0f)
#define WHEEL_CIRCUMFERENCE_M (WHEEL_DIAMETER_M * 3.1415926535f) // distance per wheel revolution
#define TRACK_WIDTH_MM 15.0f
#define TRACK_WIDTH_M (TRACK_WIDTH_MM / 1000.0f)

// Joystick hat (analog) pins.
#define JOYSTICK_X_PIN 26   // ADC0
#define JOYSTICK_Y_PIN 27   // ADC1
#define JOYSTICK_BTN_PIN 22 // Digital (active low if using typical hat)

// PWM settings for motor speed control.
#define PWM_TARGET_HZ 20000.0f
#define PWM_WRAP 999
#define PWM_CLKDIV (125000000.0f / ((PWM_WRAP + 1) * PWM_TARGET_HZ))

// Network control (robot listens for UDP drive commands).
#define CONTROL_UDP_PORT 42424
#define CONTROL_IDLE_TIMEOUT_MS 5000 // coast motors if no command arrives within this window
#define CONTROL_MESSAGE_MAX_LEN 128

// Lidar (UART) configuration.
#define LIDAR_UART_ID uart1
#define LIDAR_UART_TX_PIN 8
#define LIDAR_UART_RX_PIN 9
#define LIDAR_UART_BAUD 115200

// OLED display (SSD1306) I2C configuration.
#define OLED_I2C_INST i2c0
#define OLED_I2C_SDA_PIN 20
#define OLED_I2C_SCL_PIN 21
#define OLED_I2C_ADDR 0x3C
#define OLED_I2C_BAUD 400000

// Ultrasonic (HC-SR04) pins and timing.
#define ULTRASONIC_TRIG_PIN 18
#define ULTRASONIC_ECHO_PIN 19
#define ULTRASONIC_POLL_INTERVAL_MS 200  // how often to trigger a measurement
#define ULTRASONIC_ECHO_TIMEOUT_US 20000 // stop waiting if echo takes too long
#define ULTRASONIC_DISTANCE_UNKNOWN -1.0f
#define ULTRASONIC_BLOCK_CM 20.0f

// Collision avoidance thresholds.
#define LIDAR_FRONT_OFFSET_DEG 0     // adjust if the lidar is mounted off-axis
#define LIDAR_FRONT_HALF_SPAN_DEG 35 // +/- degrees considered "forward"
#define LIDAR_BLOCK_DISTANCE_MM 200  // stop forward motion if anything is nearer than this
#define LIDAR_BLOCK_CLEAR_MM 260     // require this clearance before allowing forward again
#define LIDAR_BLOCK_STALE_MS 250     // ignore lidar blocks if data is older than this
