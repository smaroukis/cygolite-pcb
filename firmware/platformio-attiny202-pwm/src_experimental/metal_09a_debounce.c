/*
 * Lab 4C — Button Counter with Debounce
 *
 * Target:
 *   ATtiny202
 *
 * Purpose:
 *   Read a button on PA1 and increment a counter exactly once per press.
 *
 * Pin usage:
 *   PA1 = button input with external pulldown
 *   PA3 = LED/output
 *
 * Behavior:
 *   Each valid button press increments a counter.
 *   The counter value is shown by blinking PA3.
 *
 * Example:
 *   Press 1 -> blink 1 time
 *   Press 2 -> blink 2 times
 *   Press 3 -> blink 3 times
 *
 * Key teaching idea:
 *   A mechanical button bounces.
 *   We do not count a press immediately.
 *   We wait until the input has been stable for DEBOUNCE_MS.
 */

#define F_CPU 3333333UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

#define BUTTON_PIN_bm PIN1_bm   // PA1 input
#define LED_PIN_bm    PIN3_bm   // PA3 output

#define DEBOUNCE_MS 25

static bool button_raw_is_pressed(void) {
    /*
     * External pulldown:
     *
     *   released -> PA1 = LOW
     *   pressed  -> PA1 = HIGH
     */
    return (PORTA.IN & BUTTON_PIN_bm) != 0; // boolean cast
}

static bool button_debounced_pressed_event(void) {
    static bool last_stable_state = false;
    static bool last_raw_state = false;
    static uint8_t stable_time_ms = 0;

    bool raw_state = button_raw_is_pressed();

    if (raw_state == last_raw_state) {
        if (stable_time_ms < DEBOUNCE_MS) {
            stable_time_ms++;
        }
    } else {
        stable_time_ms = 0;
        last_raw_state = raw_state;
    }

    if (stable_time_ms >= DEBOUNCE_MS) {
        if (raw_state != last_stable_state) {
            last_stable_state = raw_state;

            if (last_stable_state == true) {
                return true;  // New clean press event
            }
        }
    }

    return false;
}

static void blink_once(void) {
    PORTA.OUTSET = LED_PIN_bm;
    _delay_ms(120);

    PORTA.OUTCLR = LED_PIN_bm;
    _delay_ms(120);
}

static void show_count(uint8_t count) {
    _delay_ms(300);

    for (uint8_t i = 0; i < count; i++) {
        blink_once();
    }

    _delay_ms(500);
}

int main(void) {
    // PA1 input
    PORTA.DIRCLR = BUTTON_PIN_bm;

    // PA3 output
    PORTA.DIRSET = LED_PIN_bm;
    PORTA.OUTCLR = LED_PIN_bm;

    uint8_t press_count = 0;

    while (1) {
        if (button_debounced_pressed_event()) {
            press_count++;

            if (press_count > 5) {
                press_count = 1;
            }

            show_count(press_count);
        }

        /*
         * This delay creates a 1 ms polling interval.
         * The debounce function counts how many consecutive milliseconds
         * the raw input has stayed the same.
         */
        _delay_ms(1);
    }
}