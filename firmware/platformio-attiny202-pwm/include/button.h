#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>
#include <stdint.h>

void button_init(void);

void button_update_1ms(void);

bool button_take_pressed_event(void);

bool button_take_released_event(uint16_t *duration_ms);

bool button_is_pressed(void);

uint16_t button_current_press_duration_ms(void);

#endif