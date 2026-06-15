/*
 * Lab 4C — Button Counter with Reusable Debounce Module
 *
 * Target:
 *   ATtiny202
 *
 * Pin usage:
 *   PA1 = button input with external pulldown
 *   PA3 = LED/output
 *
 * Behavior:
 *   Each clean button press increments a counter.
 *   The count is shown as LED blinks.
 */

#define F_CPU 3333333UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

#include "button_debounce.h"

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
    return (PORTA.IN & BUTTON_PIN_bm) != 0;
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

    ButtonDebounce button;
    button_debounce_init(&button, DEBOUNCE_MS);

    uint8_t press_count = 0;

    while (1) {
        bool raw_pressed = button_raw_is_pressed();

        if (button_debounce_pressed_event(&button, raw_pressed)) {
            press_count++;

            if (press_count > 5) {
                press_count = 1;
            }

            show_count(press_count);
        }

        /*
         * The debounce module assumes this function is called every 1 ms.
         * That is why stable_time_ms increments by 1 per loop.
         */
        _delay_ms(1);
    }
}