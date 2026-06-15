#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

void button_init(void);

void button_update_1ms(void);

bool button_take_pressed_event(void);

bool button_is_pressed(void);

#endif