#ifndef BUTTON_DEBOUNCE_H
#define BUTTON_DEBOUNCE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool last_stable_state;
    bool last_raw_state;
    uint8_t stable_time_ms;
    uint8_t debounce_ms;
} ButtonDebounce;

void button_debounce_init(ButtonDebounce *button, uint8_t debounce_ms);

bool button_debounce_pressed_event(
    ButtonDebounce *button,
    bool raw_pressed_state
);

#endif