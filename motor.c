#include "motor.h"

#include <math.h>
#include "hardware/pwm.h"
#include "pico/stdlib.h"

#include "constants.h"

// Pin mapping for the L298N board (adjust if wiring changes).
typedef struct {
    uint en;
    uint in1;
    uint in2;
} motor_pins_t;

static const motor_pins_t motors[] = {
    {.en = MOTOR_A_EN_PIN, .in1 = MOTOR_A_IN1_PIN, .in2 = MOTOR_A_IN2_PIN},
    {.en = MOTOR_B_EN_PIN, .in1 = MOTOR_B_IN3_PIN, .in2 = MOTOR_B_IN4_PIN},
};

static void init_pwm_pin(uint en_pin) {
    // Configure the enable pin for PWM at the frequency/divider defined in constants.h.
    gpio_set_function(en_pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(en_pin);

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_wrap(&cfg, PWM_WRAP);
    pwm_config_set_clkdiv(&cfg, PWM_CLKDIV);
    pwm_init(slice, &cfg, true);
}

static void set_motor_outputs(const motor_pins_t *pins, bool forward, float magnitude) {
    // Clamp duty to [0,1] then set direction pins and PWM level.
    if (magnitude > 1.0f) {
        magnitude = 1.0f;
    }
    if (magnitude < 0.0f) {
        magnitude = 0.0f;
    }

    gpio_put(pins->in1, forward);
    gpio_put(pins->in2, !forward);

    uint slice = pwm_gpio_to_slice_num(pins->en);
    uint channel = pwm_gpio_to_channel(pins->en);
    uint16_t level = (uint16_t)(magnitude * PWM_WRAP);
    pwm_set_chan_level(slice, channel, level);
}

void motor_init(void) {
    // Initialize direction pins low and start PWM on both enable pins.
    for (size_t i = 0; i < sizeof motors / sizeof motors[0]; i++) {
        gpio_init(motors[i].in1);
        gpio_set_dir(motors[i].in1, GPIO_OUT);
        gpio_put(motors[i].in1, 0);

        gpio_init(motors[i].in2);
        gpio_set_dir(motors[i].in2, GPIO_OUT);
        gpio_put(motors[i].in2, 0);

        init_pwm_pin(motors[i].en);
    }
}

void motor_drive(motor_id_t motor, float speed) {
    if (motor < 0 || motor >= (int)(sizeof motors / sizeof motors[0])) {
        return;
    }

    bool forward = speed >= 0.0f;
    float magnitude = fabsf(speed);
    set_motor_outputs(&motors[motor], forward, magnitude);
}

void motor_drive_voltage(motor_id_t motor, float voltage, float supply_voltage) {
    if (supply_voltage <= 0.0f) {
        return;
    }
    // Map requested voltage to a PWM duty within allowed motor range.
    float magnitude = fabsf(voltage);
    if (magnitude < MOTOR_MIN_VOLTAGE) {
        motor_coast(motor);
        return;
    }
    if (magnitude > MOTOR_MAX_VOLTAGE) {
        magnitude = MOTOR_MAX_VOLTAGE;
    }

    float duty = magnitude / supply_voltage;
    if (duty > 1.0f) {
        duty = 1.0f;
    }

    bool forward = voltage >= 0.0f;
    set_motor_outputs(&motors[motor], forward, duty);
}

void motor_brake(motor_id_t motor) {
    if (motor < 0 || motor >= (int)(sizeof motors / sizeof motors[0])) {
        return;
    }

    gpio_put(motors[motor].in1, 1);
    gpio_put(motors[motor].in2, 1);
    set_motor_outputs(&motors[motor], true, 1.0f);
}

void motor_coast(motor_id_t motor) {
    if (motor < 0 || motor >= (int)(sizeof motors / sizeof motors[0])) {
        return;
    }

    gpio_put(motors[motor].in1, 0);
    gpio_put(motors[motor].in2, 0);
    set_motor_outputs(&motors[motor], true, 0.0f);
}

void motor_brake_all(void) {
    motor_brake(MOTOR_A);
    motor_brake(MOTOR_B);
}

void motor_coast_all(void) {
    motor_coast(MOTOR_A);
    motor_coast(MOTOR_B);
}
