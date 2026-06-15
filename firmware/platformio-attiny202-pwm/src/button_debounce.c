#include "button_debounce.h"

void button_debounce_init(ButtonDebounce *button, uint8_t debounce_ms) {
    button->last_stable_state = false;
    button->last_raw_state = false;
    button->stable_time_ms = 0;
    button->debounce_ms = debounce_ms;
}

// Event checker needs to be run every 1ms
// Assumes active HIGH button press
bool button_debounce_pressed_event(
    ButtonDebounce *button,
    bool raw_pressed_state
) {
    if (raw_pressed_state == button->last_raw_state) {
        if (button->stable_time_ms < button->debounce_ms) {
            button->stable_time_ms++;
        }
    } else {
        button->stable_time_ms = 0;
        button->last_raw_state = raw_pressed_state;
    }

    if (button->stable_time_ms >= button->debounce_ms) {
        if (raw_pressed_state != button->last_stable_state) {
            button->last_stable_state = raw_pressed_state;

            if (button->last_stable_state == true) {
                return true;  // New clean press event
            }
        }
    }

    return false;
}