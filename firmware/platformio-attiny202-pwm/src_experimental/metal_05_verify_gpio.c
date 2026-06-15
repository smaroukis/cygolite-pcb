#define F_CPU 3333333UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

// Blinks each gpio pin a pre-determined unique set of times for verification of routing.

static void blink_pin(uint8_t pin_bm, uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        PORTA.OUTSET = pin_bm;
        _delay_ms(150);

        PORTA.OUTCLR = pin_bm;
        _delay_ms(150);
    }

    _delay_ms(100);
}

int main(void) {
    // Configure all testable GPIO pins as outputs.
    // Avoid PA0 because it is UPDI/programming.
    PORTA.DIRSET = PIN1_bm | PIN2_bm | PIN3_bm | PIN6_bm | PIN7_bm;

    while (1) {
        blink_pin(PIN1_bm, 4);  // PA1 <> Package Pin 4
        blink_pin(PIN2_bm, 5);  // PA2 <> Package Pin 5
        blink_pin(PIN3_bm, 7);  // PA3 <> Package Pin 7
        blink_pin(PIN6_bm, 2);  // PA6 <> Package Pin 2
        blink_pin(PIN7_bm, 3);  // PA7 <> Package Pin 3
    }
}