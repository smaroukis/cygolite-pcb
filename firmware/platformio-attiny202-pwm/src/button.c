#include "button.h"

#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

#include "button_debounce.h"

#define BUTTON_PIN_bm PIN1_bm
#define BUTTON_DEBOUNCE_MS 25

static ButtonDebounce button_debounce;

static bool button_pressed_stable = false;
static bool button_pressed_event_pending = false;

static bool button_raw_is_pressed(void) {
    /*
     * External pulldown:
     *
     *   released -> PA1 = LOW
     *   pressed  -> PA1 = HIGH
     */
    return (PORTA.IN & BUTTON_PIN_bm) != 0;
}

// Initialize Pin1 PA1 as input
void button_init(void) {
    PORTA.DIRCLR = BUTTON_PIN_bm;
    PORTA.PIN1CTRL = 0;

    button_debounce_init(&button_debounce, BUTTON_DEBOUNCE_MS);

    button_pressed_stable = false;
    button_pressed_event_pending = false;
}

void button_update_1ms(void) {
    bool raw_pressed = button_raw_is_pressed();

    button_pressed_stable = raw_pressed;

    if (button_debounce_pressed_event(&button_debounce, raw_pressed)) {
        button_pressed_event_pending = true;
    }
}

bool button_take_pressed_event(void) {
    if (!button_pressed_event_pending) {
        return false;
    }

    button_pressed_event_pending = false;
    return true;
}

bool button_is_pressed(void) {
    return button_pressed_stable;
}