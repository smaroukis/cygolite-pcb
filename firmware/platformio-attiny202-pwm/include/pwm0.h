#ifndef PWM0_H
#define PWM0_H

#include <stdint.h>

void pwm0_init_pa3(void);

void pwm0_set_duty_8bit(uint8_t duty);

uint8_t pwm0_get_duty_8bit(void);

void pwm0_disable_pa3_output(void);

void pwm0_enable_pa3_output(void);

#endif