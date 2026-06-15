#include "button.h"

#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

#define BUTTON_PIN_bm PIN1_bm
#define BUTTON_DEBOUNCE_MS 25

// Hardware Assumptions:
// PA1 with external pulldown
// released = LOW
// pressed  = HIGH

static bool stable_pressed = false;
static bool last_raw_pressed = false;

static uint16_t raw_stable_time_ms = 0;

static bool pressed_event_pending = false;
static bool released_event_pending = false;

static uint16_t current_press_duration_ms = 0;
static uint16_t released_press_duration_ms = 0;

static bool button_raw_is_pressed(void) {
    /*
     * External pulldown wiring:
     *
     *   released -> PA1 = LOW
     *   pressed  -> PA1 = HIGH
     */
    return (PORTA.IN & BUTTON_PIN_bm) != 0;
}

void button_init(void) {
    // PA1 input
    PORTA.DIRCLR = BUTTON_PIN_bm;

    /*
     * No internal pull-up.
     * External pulldown defines released state.
     */
    PORTA.PIN1CTRL = 0;

    stable_pressed = false;
    last_raw_pressed = button_raw_is_pressed();

    raw_stable_time_ms = 0;

    pressed_event_pending = false;
    released_event_pending = false;

    current_press_duration_ms = 0;
    released_press_duration_ms = 0;
}

void button_update_1ms(void) {
    bool raw_pressed = button_raw_is_pressed();

    /*
     * Track how long the raw input has remained unchanged.
     */
    if (raw_pressed == last_raw_pressed) {
        if (raw_stable_time_ms < 0xFFFF) {
            raw_stable_time_ms++;
        }
    } else {
        last_raw_pressed = raw_pressed;
        raw_stable_time_ms = 0;
    }

    /*
     * If raw input has been stable long enough and differs from
     * the current debounced state, accept the new stable state.
     */
    if ((raw_pressed != stable_pressed) &&
        (raw_stable_time_ms >= BUTTON_DEBOUNCE_MS)) {

        stable_pressed = raw_pressed;

        if (stable_pressed) {
            /*
             * New confirmed press.
             */
            current_press_duration_ms = 0;
            pressed_event_pending = true;
        } else {
            /*
             * New confirmed release.
             * Store duration of the completed press.
             */
            released_press_duration_ms = current_press_duration_ms;
            released_event_pending = true;
            current_press_duration_ms = 0;
        }
    }

    /*
     * Count how long the button has been in confirmed pressed state.
     */
    if (stable_pressed) {
        if (current_press_duration_ms < 0xFFFF) {
            current_press_duration_ms++;
        }
    }
}

bool button_take_pressed_event(void) {
    if (!pressed_event_pending) {
        return false;
    }

    pressed_event_pending = false;
    return true;
}

bool button_take_released_event(uint16_t *duration_ms) {
    if (!released_event_pending) {
        return false;
    }

    released_event_pending = false;

    if (duration_ms != 0) {
        *duration_ms = released_press_duration_ms;
    }

    return true;
}

bool button_is_pressed(void) {
    return stable_pressed;
}

uint16_t button_current_press_duration_ms(void) {
    if (!stable_pressed) {
        return 0;
    }

    return current_press_duration_ms;
}